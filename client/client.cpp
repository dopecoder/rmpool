#include <iostream>
#include <string.h>
#include <unistd.h>
#include "rmp_client.hpp"

#define PAGE_SIZE sysconf(_SC_PAGE_SIZE)

int main(int argc, char const *argv[])
{
    double sum = 0;
    double sum2 = 0;
    double sum3 = 0;
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
    double *data2 = (double *)client.rmp_alloc(11);
    double *data3 = (double *)client.rmp_alloc(11);

    printf(" data 1 : %lx\n ", data);
    printf(" data 2 : %lx\n ", data2);

    for (int i = 0; i < ((10 * PAGE_SIZE) / sizeof(double)); i++)
    {
        data[i] = i;
    }

    for (int i = 0; i < ((10 * PAGE_SIZE) / sizeof(double)); i++)
    {
        printf("Value at %lx is %f\n", (data + i), data[i]);
        sum += data[i];
    }

    for (int i = 0; i < ((10 * PAGE_SIZE) / sizeof(double)); i++)
    {
        data2[i] = i;
    }

    for (int i = 0; i < ((10 * PAGE_SIZE) / sizeof(double)); i++)
    {
        printf("Value at %lx is %f\n", (data2 + i), data2[i]);
        sum2 += data2[i];
    }

    for (int i = 0; i < ((10 * PAGE_SIZE) / sizeof(double)); i++)
    {
        data3[i] = i;
    }

    for (int i = 0; i < ((10 * PAGE_SIZE) / sizeof(double)); i++)
    {
        printf("Value at %lx is %f\n", (data3 + i), data3[i]);
        sum3 += data3[i];
    }

    printf("Sum 1 : %f\n", sum);
    printf("Sum 2 : %f\n", sum2);
    printf("Sum 3 : %f\n", sum3);
}