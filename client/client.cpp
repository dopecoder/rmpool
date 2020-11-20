#include <iostream>
#include <string.h>
#include <unistd.h>
#include "rmp_client.hpp"

#define PAGE_SIZE sysconf(_SC_PAGE_SIZE)

int main(int argc, char const *argv[])
{
    double sum = 0;
    if (argc != 3)
    {
        std::cout << "Invalid args!" << std::endl;
        std::cout << "Usage: ./server <ip> <port>" << std::endl;
        return 1;
    }

    std::cout << argv[1] << std::endl;
    std::cout << atoi(argv[2]) << std::endl;

    rmp::config cnf{
        argv[1],
        atoi(argv[2])};

    rmp::Client client(cnf);

    client.rmp_init();

    double *data = (double *)client.rmp_alloc(10);

    // char *pg = (char *)malloc(4096);
    // memset(data, 1, 8192 / 4);
    // data[0] = 'H';
    // data[1] = 'E';
    // data[2] = 'L';
    // data[3] = 'L';
    // data[4] = 'O';

    for (int i = 0; i < (2 * PAGE_SIZE) / sizeof(double); i++)
    {
        data[i] = 100.0;
    }

    for (int i = 0; i < (2 * PAGE_SIZE) / sizeof(double); i++)
    {
        printf("Value at %lx is %f\n", (data + i), data[i]);
        sum += data[i];
    }

    printf("Sum : %f\n", sum);

    // client.rmp_write(hndl, 0, pg);

    // char *return_page = (char *)malloc(4096);
    // client.rmp_read(hndl, 0, return_page);

    while (1)
    {
        /* code */
    }
}