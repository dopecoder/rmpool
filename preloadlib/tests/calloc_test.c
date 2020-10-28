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
    void *calloc_ptrs[limit];

    printf("%s\n", "calloc_test: (main) called");

    for (i = 0; i < limit; ++i)
    {
        calloc_ptrs[i] = calloc(10000 * i, sizeof(char));
    }

    // for (i = 0; i < limit; ++i)
    // {
    //     free(calloc_ptrs[i]);
    // }
    printf("%s\n", "calloc_test: (main) success, exiting");
    return 0;
}