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

    printf(" RMP_INIT returned %d\n ", client.rmp_init());

    double *data = (double *)client.rmp_alloc(11);

    for (int i = 0; i < ((10 * PAGE_SIZE) / sizeof(double)); i++)
    {
        data[i] = i;
    }

    for (int i = 0; i < ((10 * PAGE_SIZE) / sizeof(double)); i++)
    {
        printf("Value at %lx is %f\n", (data + i), data[i]);
        sum += data[i];
    }

    printf("Sum : %f\n", sum);
}