#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void print(const char *str)
{
    // const char msg[] = "preloadlib.so: (malloc) called\n";
    write(STDOUT_FILENO, str, strlen(str));
}

int main(int argc, char *argv[])
{
    // int i;
    // const int limit = 1;
    // void *ptr[limit];
    print("START\n");
    void *result = malloc(1024);
    print("END\n");
    // printf("Res : %lx\n", result);

    // printf("%s\n", "malloc_test: (main) called");
    // for (i = 0; i < limit; ++i)
    // {
    //     ptr[i] = malloc(10000 * i);
    // }
    // for (i = 0; i < limit; ++i)
    // {
    //     free(ptr[i]);
    // }
    return 0;
}