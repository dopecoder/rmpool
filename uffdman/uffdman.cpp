#include <sys/types.h>
#include <linux/userfaultfd.h>
#include <pthread.h>
#include <inttypes.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <poll.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <unordered_map>
#include <iostream>

#include "uffdman.hpp"

#ifndef DEBUG_PRINTS
#define DEBUG_PRINTS 0
#endif

// #if DEBUG_PRINTS
// #define err(msg) perror(msg)
// #define cout(exp...) printf(exp);
// #else
// #define err(msg)
// #define cout(exp...)
// #endif

#if DEBUG_PRINTS
#define err(msg) perror(msg)
#define print(exp...) printf(exp);
#else
#define err(msg)
#define print(exp...)
#endif

#define PAGE_SIZE sysconf(_SC_PAGE_SIZE)
#define PGROUNDDOWN(a) (((a)) & ~(PAGE_SIZE - 1))

using namespace std;

bool uffd_initialized = false;

static unordered_map<unsigned long, pthread_t> *addr_tid_map;

// Double sided maps for starting address of a region to uffd
static unordered_map<unsigned long, long> *start_addr_uffd_map;
static unordered_map<long, unsigned long> *uffd_start_addr_map;

// Maps to maintain prev resolved pages for lazy page resolving
static unordered_map<long, unsigned long> *uffd_prev_resolved_addr;
static unordered_map<long, unsigned long> *uffd_prev_resolved_op;

static void (*page_resolver)(void *start_addr, void *faulting_addr, int is_write, void *page);

void uffdman_init()
{

    if (!uffd_initialized)
    {
        uffd_prev_resolved_addr = new unordered_map<long, unsigned long>();
        uffd_prev_resolved_op = new unordered_map<long, unsigned long>();
        addr_tid_map = new unordered_map<unsigned long, pthread_t>();
        start_addr_uffd_map = new unordered_map<unsigned long, long>();
        uffd_start_addr_map = new unordered_map<long, unsigned long>();
        uffd_initialized = true;
    }
}

void uffdman_destroy()
{
    delete addr_tid_map;
    delete start_addr_uffd_map;
    delete uffd_start_addr_map;
    delete uffd_prev_resolved_addr;
    delete uffd_prev_resolved_op;
}

/*

	Off-by-one Invalidation: If a pagefault occurs, then the faulting address and operation (read/write)
	is saved, so that it can be invalidated when the next pagefault occurs. 

	This is useful because a new pagefault means access to new address, so we can invalidate the previous 
	one and if the recently invalidated address is accessed then that will cause a pagefault which invalidates
	the one resolved before and so on.
	
	This is done to solve two problems:
		1. A pagefault (userfault) won't occur for subsquent reads/writes once it is resolved/handled.
		2. In a pagefault due to write we won't get the data (part of faulting instruction) that is 
		   being written in the userfault handler.

	Constraints:
		1. Atmost two pages of memory is required.
		2. The memory of entity which is resoving pages will be behind by one memory address in terms of 
		   consistency. Since the name Off-by-one.

*/

// template <class Key,
//           class T,
//           class Hash,
//           class Pred,
//           class Alloc>
// // template <class K>
// static bool exists(std::unordered_map<Key, T, Hash, Pred, Alloc> &m, Key key)
// {
//     auto got = m.find(key);

//     if (got == m.end())
//         return false;
//     return true;
// }

static void invalidate_prev_resolved_page(long uffd)
{
    auto prev_resolved_addr_exists = uffd_prev_resolved_addr->find(uffd);
    auto prev_resolved_op_exists = uffd_prev_resolved_op->find(uffd);
    auto region_start_addr_exists = uffd_start_addr_map->find(uffd);

    if (prev_resolved_addr_exists != uffd_prev_resolved_addr->end() &&
        prev_resolved_op_exists != uffd_prev_resolved_op->end() &&
        region_start_addr_exists != uffd_start_addr_map->end())
    {

        unsigned long prev_resolved_addr = uffd_prev_resolved_addr->at(uffd);
        int prev_resolved_op = uffd_prev_resolved_op->at(uffd);
        void *region_start_addr = (void *)uffd_start_addr_map->at(uffd);

        // Invalidate previous page after resolving it
        void *prev_page = (void *)prev_resolved_addr;

        print("Prev Resolved Address = %" PRIx64 "; ", prev_resolved_addr);

        if (prev_resolved_op)
        {
            // Resolve only if write
            print("invalidate_prev_resolved_page : Saving page to server as the page is dirty\n");
            page_resolver(region_start_addr, (void *)prev_resolved_addr, prev_resolved_op, prev_page);
        }

        madvise((void *)PGROUNDDOWN(prev_resolved_addr), PAGE_SIZE, MADV_DONTNEED);
    }
}

static void handle_unmap_event(long uffd)
{
    void *region_start_addr = (void *)uffd_start_addr_map->at(uffd);
    uffdman_unregister_region(region_start_addr);
}

static void *fault_handler_thread(void *arg)
{
    print("FAULT HANDLER STARTED\n");
    static struct uffd_msg msg; /* Data read from userfaultfd */
    static int fault_cnt = 0;   /* Number of faults so far handled */
    long uffd;                  /* userfaultfd file descriptor */
    static void *page = NULL;
    struct uffdio_copy uffdio_copy;
    ssize_t nread;

    uffd = (long)arg;

    /* Create a page that will be copied into the faulting region */

    if (page == NULL)
    {
        page = (void *)mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (page == MAP_FAILED)
            print("mmap in userfault handler failed\n");
    }

    /* Loop, handling incoming events on the userfaultfd
       file descriptor */

    while (true)
    {
        /* See what poll() tells us about the userfaultfd */

        struct pollfd pollfd;
        int nready;
        pollfd.fd = uffd;
        pollfd.events = POLLIN;
        nready = poll(&pollfd, 1, -1);
        if (nready == -1)
            break;
        print("\nfault_handler_thread():\n");
        print("    poll() returns: nready = %d; POLLIN = %d; POLLERR = \n", nready, (int)((pollfd.revents & POLLIN) != 0), (int)((pollfd.revents & POLLERR) != 0));
        /* Read an event from the userfaultfd */

        nread = read(uffd, &msg, sizeof(msg));
        if (nread == 0)
        {
            print("EOF on userfaultfd!\n");
            break;
        }

        if (nread == -1)
            break;

        if (msg.event == UFFD_EVENT_UNMAP)
        {
            // TODO: Handle arbitrary unmap event
            continue;
        }
        /* We expect only one kind of event; verify that assumption */

        if (msg.event != UFFD_EVENT_PAGEFAULT)
        {
            err("Unexpected event on userfaultfd");
            break;
        }

        /* Display info about the page-fault event */
        print("    UFFD_EVENT_PAGEFAULT event: ");
        print("uffd = %d; ", uffd);
        print("flags = %" PRIx64 "; ", msg.arg.pagefault.flags);
        print("address = %" PRIx64 "; ", msg.arg.pagefault.address);
        print("Rounded address = %" PRIx64 "\n", PGROUNDDOWN(msg.arg.pagefault.address));

        fault_cnt++;

        void *faulting_addr = (void *)msg.arg.pagefault.address;
        int faulting_op = msg.arg.pagefault.flags & UFFD_PAGEFAULT_FLAG_WRITE;

        invalidate_prev_resolved_page(uffd);

        if (!faulting_op)
        { // Resolve current pauge fault only if read
            print("Calling rmp_page_resolver with faulting_addr : %" PRIx64 " and Rounded : %" PRIx64 "\n", faulting_addr, PGROUNDDOWN((unsigned long)faulting_addr));
            void *region_start_addr = (void *)uffd_start_addr_map->at(uffd);
            page_resolver(region_start_addr, faulting_addr, faulting_op, page);
        }

        /* 
			How to handle pagefault due to a write?
			
			Example:
				addr = mmap(...); // demand paged mapping
				addr[offset] = some_data;
							   |-------| -> how to get this in page fault?					

			Solution: Off-by-one Invalidation. Meaning, handle the write during next pagefault. If no page
			fault occurs after resolving a write then invalidate during unregistration.

		*/

        /* Copy the page pointed to by 'page' into the faulting
           region. Vary the contents that are copied in, so that it
           is more obvious that each fault is handled separately. */
        uffdio_copy.src = (unsigned long)page;

        /* We need to handle page faults in units of pages(!).
           So, round faulting address down to page boundary */

        uffdio_copy.dst = PGROUNDDOWN((unsigned long)faulting_addr);
        uffdio_copy.len = PAGE_SIZE;
        uffdio_copy.mode = 0;
        uffdio_copy.copy = 0;
        if (ioctl(uffd, UFFDIO_COPY, &uffdio_copy) == -1)
            break;
        print("\t(uffdio_copy.copy returned %lld\n", uffdio_copy.copy);

        // (*uffd_prev_resolved_addr)[uffd] = PGROUNDDOWN((unsigned long)faulting_addr);
        // Check and Insert
        auto prev_resolved_addr_it = uffd_prev_resolved_addr->find(uffd);
        if (prev_resolved_addr_it != uffd_prev_resolved_addr->end())
        {
            prev_resolved_addr_it->second = PGROUNDDOWN((unsigned long)faulting_addr);
        }
        else
        {
            uffd_prev_resolved_addr->insert({uffd, PGROUNDDOWN((unsigned long)faulting_addr)});
        }

        // Check and Insert
        auto prev_resolved_op_it = uffd_prev_resolved_op->find(uffd);
        if (prev_resolved_op_it != uffd_prev_resolved_op->end())
        {
            prev_resolved_op_it->second = faulting_op;
        }
        else
        {
            uffd_prev_resolved_op->insert({uffd, faulting_op});
        }
    }

    return 0;
}

void uffdman_register_page_resolver(void (*resolver)(void *start_addr, void *faulting_addr, int is_write, void *page))
{
    page_resolver = resolver;
}

void uffdman_unregister_page_resolver()
{
    page_resolver = NULL;
}

int uffdman_register_region(void *addr, unsigned long n_pages)
{
    long uffd;         /* userfaultfd file descriptor */
    unsigned long len; /* Length of region handled by userfaultfd */
    struct uffdio_api uffdio_api;
    struct uffdio_register uffdio_register;
    pthread_t thrd_id;

    len = n_pages * PAGE_SIZE;

    /* Create and enable userfaultfd object */

    uffd = syscall(__NR_userfaultfd, O_CLOEXEC | O_NONBLOCK);
    if (uffd == -1)
    {
        err("Error in userfaultfd syscall");
        return -1;
    }

    uffdio_api.api = UFFD_API;
    uffdio_api.features = 0;
    if (ioctl(uffd, UFFDIO_API, &uffdio_api) == -1)
    {
        err("Error in ioctl UFFDIO_API");
        return -1;
    }

    uffdio_register.range.start = (unsigned long)addr;
    uffdio_register.range.len = len;
    uffdio_register.mode = UFFDIO_REGISTER_MODE_MISSING;
    if (ioctl(uffd, UFFDIO_REGISTER, &uffdio_register) == -1)
    {
        err("Error in ioctl UFFDIO_REGISTER");
        return -1;
    }

    /* Create a thread that will process the userfaultfd events */
    int out = pthread_create(&thrd_id, NULL, fault_handler_thread, (void *)uffd);
    if (out != 0)
    {
        errno = thrd_id;
        err("Error while creating fault handler thread");
        return -1;
    }

    addr_tid_map->insert({(unsigned long)addr, thrd_id});
    start_addr_uffd_map->insert({(unsigned long)addr, uffd});
    uffd_start_addr_map->insert({uffd, (unsigned long)addr});
    return 0;
}

void uffdman_unregister_region(void *addr)
{
    unsigned long ul_addr = (unsigned long)addr;
    auto start_addr_uffd_exists = start_addr_uffd_map->find(ul_addr);
    if (start_addr_uffd_exists != start_addr_uffd_map->end())
    {
        long uffd = start_addr_uffd_map->at(ul_addr);
        invalidate_prev_resolved_page(uffd);

        start_addr_uffd_map->erase(ul_addr);
        uffd_start_addr_map->erase(uffd);

        uffd_prev_resolved_addr->erase(uffd);
        uffd_prev_resolved_op->erase(uffd);

        auto addr_tid_exists = addr_tid_map->find(ul_addr);
        if (addr_tid_exists != addr_tid_map->end())
        {
            pthread_t tid = addr_tid_map->at(ul_addr);
            addr_tid_map->erase(tid);
            pthread_cancel(tid);
        }
    }
}
