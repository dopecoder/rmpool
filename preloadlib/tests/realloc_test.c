#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void print(const char *str)
{
    write(STDOUT_FILENO, str, strlen(str));
}

int main(int argc, char *argv[])
{
    int i;
    const int limit = 5;
    void *malloc_ptrs[limit];

    printf("%s\n", "realloc_test: (main) called");
    for (i = 0; i < limit; ++i)
    {
        malloc_ptrs[i] = malloc(10000 * i);
    }

    for (i = 0; i < limit; ++i)
    {
        malloc_ptrs[i] = realloc(malloc_ptrs[i], 20000 * i);
    }

    for (i = 0; i < limit; ++i)
    {
        free(malloc_ptrs[i]);
    }
    printf("%s\n", "realloc_test: (main) success, exiting");
    return 0;
}