#define _GNU_SOURCE
#include <sys/types.h>
#include <stdio.h>
#include <linux/userfaultfd.h>
#include <pthread.h>
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

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); } while (0)

/*
	TODO:
		- remove comments
		- remove unnecessary prints logging
		- convert C to C++ where ever possible
*/

static int page_size = -1;

static unordered_map<unsigned long, pthread_t> addr_tid_map;
static unordered_map<unsigned long, long> addr_uffd_map;
static unordered_map<long, unsigned long> uffd_addr_map;

static void (*page_resolver) (char* start_addr, char* faulting_addr, long page_size, int is_write, char** page);

static void* fault_handler_thread(void *arg)
{
    static struct uffd_msg msg;   /* Data read from userfaultfd */
    static int fault_cnt = 0;     /* Number of faults so far handled */
    long uffd;                    /* userfaultfd file descriptor */
    static char *page = NULL;
    struct uffdio_copy uffdio_copy;
    ssize_t nread;

    uffd = (long) arg;

    /* Create a page that will be copied into the faulting region */
/*
    if (page == NULL) {
        page = mmap(NULL, page_size, PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (page == MAP_FAILED)
            errExit("mmap");
    }
*/
    /* Loop, handling incoming events on the userfaultfd
       file descriptor */

    for (;;) {

        /* See what poll() tells us about the userfaultfd */

        struct pollfd pollfd;
        int nready;
        pollfd.fd = uffd;
        pollfd.events = POLLIN;
        nready = poll(&pollfd, 1, -1);
        if (nready == -1)
            errExit("poll");

        printf("\nfault_handler_thread():\n");
        printf("    poll() returns: nready = %d; "
                "POLLIN = %d; POLLERR = %d\n", nready,
                (pollfd.revents & POLLIN) != 0,
                (pollfd.revents & POLLERR) != 0);

        /* Read an event from the userfaultfd */

        nread = read(uffd, &msg, sizeof(msg));
        if (nread == 0) {
            printf("EOF on userfaultfd!\n");
            exit(EXIT_FAILURE);
        }

        if (nread == -1)
            errExit("read");

        /* We expect only one kind of event; verify that assumption */

        if (msg.event != UFFD_EVENT_PAGEFAULT) {
            fprintf(stderr, "Unexpected event on userfaultfd\n");
            exit(EXIT_FAILURE);
        }

        /* Display info about the page-fault event */

        printf("    UFFD_EVENT_PAGEFAULT event: ");
        printf("flags = %llx; ", msg.arg.pagefault.flags);
        printf("address = %llx\n", msg.arg.pagefault.address);

        /* Copy the page pointed to by 'page' into the faulting
           region. Vary the contents that are copied in, so that it
           is more obvious that each fault is handled separately. */
/*
        memset(page, 'A' + fault_cnt % 20, page_size);
*/
        fault_cnt++;

        char* region_start_addr = (char*) uffd_addr_map[uffd];

        page_resolver(region_start_addr, msg.arg.pagefault.address, msg.arg.pagefault.flags & UFFD_PAGEFAULT_FLAG_WRITE, &page);
		
		/* 
			TODO How to handle pagefault due to a write?
			
			Example:
				addr = mmap(...); // demand paged mapping
				addr[offset] = some_data;
							   |-------| -> how to get this in page fault?					
		*/

        
        uffdio_copy.src = (unsigned long) page;

        /* We need to handle page faults in units of pages(!).
           So, round faulting address down to page boundary */

        uffdio_copy.dst = (unsigned long) msg.arg.pagefault.address &
            ~(page_size - 1);
        uffdio_copy.len = page_size;
        uffdio_copy.mode = 0;
        uffdio_copy.copy = 0;
        if (ioctl(uffd, UFFDIO_COPY, &uffdio_copy) == -1)
            errExit("ioctl-UFFDIO_COPY");

        printf("        (uffdio_copy.copy returned %lld)\n",
                uffdio_copy.copy);
    }
}

void register_uffd_page_resolver(void (*resolver) (char* start_addr, char* faulting_addr, long page_size, int is_write, char** page)) {
	page_resolver = resolver;
}

void unregister_uffd_page_resolver() {
	callback = NULL;
}

void register_uffd_region(char *addr, unsigned long n_pages)
{
    long uffd;          /* userfaultfd file descriptor */
//    char *addr;         /* Start of region handled by userfaultfd */
    unsigned long len;  /* Length of region handled by userfaultfd */
    pthread_t thr;      /* ID of thread that handles page faults */
    struct uffdio_api uffdio_api;
    struct uffdio_register uffdio_register;
    pthread_t thrd_id;
/*
    if (argc != 2) {
        fprintf(stderr, "Usage: %s num-pages\n", argv[0]);
        exit(EXIT_FAILURE);
    }
*/
    if(page_size==-1) page_size = sysconf(_SC_PAGE_SIZE);

    len = n_pages * page_size;

    /* Create and enable userfaultfd object */

    uffd = syscall(__NR_userfaultfd, O_CLOEXEC | O_NONBLOCK);
    if (uffd == -1)
        errExit("userfaultfd");

    uffdio_api.api = UFFD_API;
    uffdio_api.features = 0;
    if (ioctl(uffd, UFFDIO_API, &uffdio_api) == -1)
        errExit("ioctl-UFFDIO_API");

    /* Create a private anonymous mapping. The memory will be
       demand-zero paged--that is, not yet allocated. When we
       actually touch the memory, it will be allocated via
       the userfaultfd. */
/*
    addr = mmap(NULL, len, PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (addr == MAP_FAILED)
        errExit("mmap");

    printf("Address returned by mmap() = %p\n", addr);
*/
    /* Register the memory range of the mapping we just created for
       handling by the userfaultfd object. In mode, we request to track
       missing pages (i.e., pages that have not yet been faulted in). */

    uffdio_register.range.start = (unsigned long) addr;
    uffdio_register.range.len = len;
    uffdio_register.mode = UFFDIO_REGISTER_MODE_MISSING;
    if (ioctl(uffd, UFFDIO_REGISTER, &uffdio_register) == -1)
        errExit("ioctl-UFFDIO_REGISTER");

    /* Create a thread that will process the userfaultfd events */

    thrd_id = pthread_create(&thr, NULL, fault_handler_thread, (void *) uffd);
    if (thrd_id != 0) {
        errno = thrd_id;
        errExit("pthread_create");
    }

    addr_tid_map[(unsigned long) addr] = thrd_id;
	addr_uffd_map[(unsigned long) addr] = uffd;
	uffd_addr_map[uffd] = (unsigned long) addr;
    /* Main thread now touches memory in the mapping, touching
       locations 1024 bytes apart. This will trigger userfaultfd
       events for all pages in the region. */
/*
    int l;
    l = 0xf;    
*/
    /* 	Ensure that faulting address is not on a page
   		boundary, in order to test that we correctly
       	handle that case in fault_handling_thread() */
/*
    while (l < len) {
        char c = addr[l];
        printf("Read address %p in main(): ", addr + l);
        printf("%c\n", c);
        l += 1024;
        usleep(100000);         // Slow things down a little 
    }

    exit(EXIT_SUCCESS);
*/
}

void unregister_uffd_regoin(char* addr) {
	unsigned long ul_addr = (unsigned long) addr;
	pthread_t tid = addr_tid_map[ul_addr];
	pthread_cancel(tid);
	addr_tid_map.erase(tid);
	long uffd = addr_uffd_map[ul_addr];
	addr_uffd_map.erase(ul_addr);
	uffd_addr_map.erase(uffd);

}
