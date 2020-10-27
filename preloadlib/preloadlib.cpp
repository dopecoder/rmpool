// #ifndef _GNU_SOURCE
// #define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/mman.h>

#include "rmp.hpp"

#define PAGE_SIZE sysconf(_SC_PAGE_SIZE)

static void *(*real_malloc)(size_t) = NULL;
static void *(*real_realloc)(void *, size_t) = NULL;
static void *(*real_calloc)(size_t, size_t) = NULL;
static void (*real_free)(void *) = NULL;

static int alloc_init_pending = 0;
static int alloc_local = 0;

int rmp_fd = -1;

void print(const char *str)
{
    // const char msg[] = "preloadlib.so: (malloc) called\n";
    write(STDOUT_FILENO, str, strlen(str));
}

// Initialize the rmp connection to server and get a fd
static void rmp_conn_init(void)
{
    print("preloadlib.so: (init) called\n");
    ConnectionConfig config(
        "127.0.0.1",
        "6767",
        "");

    alloc_local = 1;
    print("preloadlib.so: (init) rmp_init Started\n");
    rmp_fd = rmp_init(config);
    print("preloadlib.so: (init) rmp_init finished\n");
    alloc_local = 0;
    print("preloadlib.so: (init) exiting\n");
}

/* Load original allocation routines at first use */
static void alloc_init(void)
{
    print("preloadlib.so: (alloc_init) called\n");
    alloc_init_pending = 1;
    real_malloc = (void *(*)(size_t))dlsym(RTLD_NEXT, "malloc");
    real_realloc = (void *(*)(void *, size_t))dlsym(RTLD_NEXT, "realloc");
    real_calloc = (void *(*)(size_t, size_t))dlsym(RTLD_NEXT, "calloc");
    real_free = (void (*)(void *))dlsym(RTLD_NEXT, "free");
    if (!real_malloc || !real_realloc || !real_calloc || !real_free)
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

    alloc_init_pending = 0;
    print("preloadlib.so: (alloc_init) exiting\n");
}

static int check_conn(void)
{
    print("preloadlib.so: (check_conn) called\n");
    // print("preloadlib.so: (check_conn) called\n");
    if (!real_malloc)
    {
        alloc_init();
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

// void free(void *ptr)
// {
//     // if (alloc_init_pending)
//     // {
//     //     print("preloadlib.so: free internal\n", stderr);
//     //     /* Ignore 'free' during initialization and ignore potential mem leaks
//     //  * On the tested system, this did not happen
//     //  */
//     //     return;
//     // }

//     print("preloadlib.so: (free) called\n");
//     if (check_conn())
//     {
//         rmp_free((char *)ptr);
//     }
//     // else
//     // {
//     //     real_free(ptr);
//     // }

//     // for (size_t i = 0; i < zalloc_cnt; i++)
//     // {
//     //     if (zalloc_list[i] == ptr)
//     //     {
//     //         /* If dlsym cleans up its dynamic memory allocated with zalloc_internal,
//     //    * we intercept and ignore it, as well as the resulting mem leaks.
//     //    * On the tested system, this did not happen
//     //    */
//     //         return;
//     //     }
//     // }
//     // real_free(ptr);
//     print("preloadlib.so: (free) exiting\n", stderr);
// }

void *malloc(size_t size)
{
    print("preloadlib.so: (malloc) called\n");

    void *result;

    // if (alloc_init_pending)
    // {
    //     extern void *__libc__malloc(size_t);
    //     return __libc__malloc(size);
    // }
    // if (alloc_init_pending)
    // {
    //     print("preloadlib.so: malloc internal\n", stderr);
    //     return zalloc_internal(size);
    // }

    // if (real_malloc == NULL)
    // {

    //     alloc_init();
    // }

    if (alloc_local)
    {
        print("preloadlib.so: (malloc) local allocation started\n");
        if (real_malloc == NULL)
        {
            alloc_init();
            print("preloadlib.so: (malloc) real_malloc is NULL\n");
            return NULL;
        }
        result = real_malloc(size);
        print("preloadlib.so: (malloc) local allocation done\n");
        return result;
    }

    if (check_conn())
    {
        //call rmp_alloc
        print("preloadlib.so: (malloc) remote allocation\n");

        alloc_local = 1;
        long npages = ceil((float)size / (float)PAGE_SIZE);
        printf("DIV : %u\n", npages);
        printf("Size : %ld, PAGE SIZE : %ld\n", size, PAGE_SIZE);
        // print("preloadlib.so: (malloc) Allocating %ud pages\n", npages);
        result = (void *)rmp_alloc(npages);
        print("preloadlib.so: (malloc) remote allocation finished\n");
        // alloc_local = 0;
    }
    // else
    // {
    //     result = real_malloc(size);
    // }

    //fprintf(stderr, "preloadlib.so: malloc(0x%zx) = %p\n", size, result);
    print("preloadlib.so: (malloc) exiting\n");
    return result;
}

// void *calloc(size_t nmemb, size_t size)
// {
//     print("preloadlib.so: (calloc) called\n");
//     void *result;
//     // if (alloc_init_pending)
//     // {
//     //     print("preloadlib.so: calloc internal\n", stderr);
//     //     /* Be aware of integer overflow in nmemb*size.
//     //  * Can only be triggered by dlsym */
//     //     return zalloc_internal(nmemb * size);
//     // }
//     if (check_conn())
//     {
//         //call rmp_alloc
//         //set everything to zero
//         uint npages = ceil((nmemb * size) / PAGE_SIZE);
//         // print("preloadlib.so: (calloc) Allocating %ud pages\n", npages);
//         result = (void *)rmp_alloc(npages);
//         // todo(dopecoder) : Make the pages zero in nmemb size
//     }
//     // else
//     // {
//     //     result = real_calloc(nmemb, size);
//     // }
//     print("preloadlib.so: (calloc) exiting\n", stderr);
//     return result;
// }

// We dont have a way to do this yet
// void *realloc(void *ptr, size_t size)
// {
//     void *result;
//     if (alloc_init_pending)
//     {
//         print("preloadlib.so: realloc internal\n", stderr);
//         if (ptr)
//         {
//             print("preloadlib.so: realloc resizing not supported\n", stderr);
//             exit(1);
//         }
//         return zalloc_internal(size);
//     }

//     if (check_conn())
//     {
//         // We dont have a way to do this yet
//     }
//     else
//     {
//         result = real_malloc(size);
//     }
//     return result;
// }