#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void print(const char *str)
{
    write(STDOUT_FILENO, str, strlen(str));
}

static int foo()
{
    int n = 4096 * 5;
    int sum = 0;

    int *p = (int *)malloc(n);

    // memset(p, 1, n);
    for (int i = 0; i < n / sizeof(int); i++)
    {
        // write(STDOUT_FILENO, i + '0', strlen(i + '0'));
        // write(STDOUT_FILENO, '\n', 1);
        printf("%d\n", i);
        p[i] = i;
    }

    for (int i = 0; i < n / sizeof(int); i++)
    {
        sum += p[i];
    }
    return sum;
}

int main(int argc, char *argv[])
{
    // int i;
    // const int limit = 100;
    // void *malloc_ptrs[limit];
    // getchar();
    // printf("Hello World!\n");

    printf("%d\n", foo());
#if 0
    printf("%s\n", "malloc_test: (main) called");
    for (i = 0; i < limit; ++i)
    {
        malloc_ptrs[i] = malloc(10000 * i);
    }

    for (i = 0; i < limit; ++i)
    {
        *((char *)malloc_ptrs[i]) = 'H';
    }

    for (i = 0; i < limit; ++i)
    {
        free(malloc_ptrs[i]);
    }
    printf("%s\n", "malloc_test: (main) success, exiting");
#endif
    return 0;
}