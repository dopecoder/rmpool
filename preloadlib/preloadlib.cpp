// #ifndef _GNU_SOURCE
// #define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <pthread.h>
#include <mutex>

#include "rmp_client.hpp"

// #if DEBUG_PRINTS
// #define err(msg) perror(msg)
// #define cout(exp) cout << exp;
// #else
// #define err(msg)
// #define cout(exp)
// #endif

#define PAGE_SIZE sysconf(_SC_PAGE_SIZE)

static void *(*stdlib_malloc)(size_t) = NULL;
static void *(*stdlib_realloc)(void *, size_t) = NULL;
static void *(*stdlib_calloc)(size_t, size_t) = NULL;
static void (*stdlib_free)(void *) = NULL;

static int preloadlib_init_pending = 0;
static int do_local_allocation = 0;

int rmp_fd = -1;
pthread_t mainthread_id;
rmp::Client *client;
extern std::mutex fault_handler_cr;

void print(const char *str)
{
    // #if DEBUG_PRINTS
    // write(STDOUT_FILENO, str, strlen(str));
    // #endif
}

// void myprintf(const char *__restrict__ __format, ...)
// {
//     va_list argptr;
//     va_start(argptr, __format);
//     vfprintf(stderr, __format, argptr);
//     va_end(argptr);
// }

// Initialize the rmp connection to server and get a fd
static void rmp_conn_init(void)
{
    print("preloadlib.so: (init) called\n");

    // rmp::config conf{
    //     "127.0.0.1",
    //     6767};

    rmp::config conf{
        "127.0.0.1",
        6768};

    std::lock_guard<std::mutex> guard(fault_handler_cr);
    do_local_allocation = 1;
    mainthread_id = pthread_self();
    print("preloadlib.so: (init) rmp_init Started\n");
    client = new rmp::Client(conf);
    rmp_fd = client->rmp_init();
    // printf("RMP_FD IS %d\n", rmp_fd);
    print("preloadlib.so: (init) rmp_init finished\n");
    do_local_allocation = 0;
    print("preloadlib.so: (init) exiting\n");
}

/* Load original allocation routines at first use */
static void preloadlib_init(void)
{
    print("preloadlib.so: (preloadlib_init) called\n");
    preloadlib_init_pending = 1;
    stdlib_malloc = (void *(*)(size_t))dlsym(RTLD_NEXT, "malloc");
    stdlib_realloc = (void *(*)(void *, size_t))dlsym(RTLD_NEXT, "realloc");
    stdlib_calloc = (void *(*)(size_t, size_t))dlsym(RTLD_NEXT, "calloc");
    stdlib_free = (void (*)(void *))dlsym(RTLD_NEXT, "free");
    if (!stdlib_malloc || !stdlib_realloc || !stdlib_calloc || !stdlib_free)
    {
        print("preloadlib.so: Unable to hook allocation!\n");
        print(dlerror());
        exit(1);
    }
    else
    {
        print("preloadlib.so: Successfully hooked\n");
    }

    // Initialize the rmp connection
    // rmp_conn_init();

    preloadlib_init_pending = 0;
    print("preloadlib.so: (preloadlib_init) exiting\n");
}

static int check_conn(void)
{
    print("preloadlib.so: (check_conn) called\n");
    // print("preloadlib.so: (check_conn) called\n");
    if (!stdlib_malloc)
    {
        preloadlib_init();
    }

    if (rmp_fd == -1)
    {
        rmp_conn_init();
    }

    if (rmp_fd != -1)
    {
        print("rmp_fd is initialized!\n");
    }
    else
    {
        print("rmp_fd is uninitialized!\n");
    }
    print("preloadlib.so: (check_conn) exiting\n");
    return rmp_fd != -1;
    // return false;
}

void free(void *ptr)
{
    print("preloadlib.so: (free) called\n");
    // if (do_local_allocation == 1)
    // {
    //     print("Local free\n");
    //     // stdlib_free(ptr);
    // }
    // else
    // {
    //     if (check_conn())
    //     {
    //         print("Remote free\n");
    //         do_local_allocation = 1;
    //         rmp_free((char *)ptr);
    //         do_local_allocation = 0;
    //     }
    // }
    print("preloadlib.so: (free) exiting\n");
}

void *local_malloc(size_t size)
{
    void *result;
    print("preloadlib.so: (malloc) local allocation started\n");
    if (stdlib_malloc == NULL)
    {
        preloadlib_init();
        print("preloadlib.so: (malloc) stdlib_malloc is NULL\n");
        return NULL;
    }
    result = stdlib_malloc(size);
    print("preloadlib.so: (malloc) local allocation done\n");
    return result;
}

void *malloc(size_t size)
{
    print("preloadlib.so: (malloc) called\n");

    void *result;
    if (do_local_allocation)
    {
        return local_malloc(size);
    }

    if (check_conn())
    {

        if (pthread_self() != mainthread_id)
        {
            return local_malloc(size);
        }

        std::lock_guard<std::mutex> guard(fault_handler_cr);
        //call rmp_alloc
        print("preloadlib.so: (malloc) remote allocation\n");
        do_local_allocation = 1;
        long npages = ceil((float)size / (float)PAGE_SIZE);
        // printf("Size : %ld, PAGE SIZE : %ld, Pages : %ld\n", size, PAGE_SIZE, npages);
        result = (void *)client->rmp_alloc(npages);
        fault_handler_cr.unlock();
        print("preloadlib.so: (malloc) remote allocation finished\n");
        do_local_allocation = 0;
        return result;
    }
    // else
    // {
    //     result = stdlib_malloc(size);
    // }

    //fprintf(stderr, "preloadlib.so: malloc(0x%zx) = %p\n", size, result);
    print("preloadlib.so: (malloc) failed\n");
    return NULL;
}