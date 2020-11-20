
#include <sys/socket.h>
#include <sys/mman.h>
#include <iostream>
#include <unordered_map>
#include <arpa/inet.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "uffdman.hpp"
#include "rmp_client.hpp"

#define PAGE_SIZE sysconf(_SC_PAGE_SIZE)

int socketfd;
// rmp::handle hndl;
bool connected = false;
std::unordered_map<ul, rmp::handle> *addr_hndl_map;
std::unordered_map<ul, ul> *addr_npages_map;

void rmp_write_page(rmp::handle hd, ul offset, void *page)
{
    printf("entered (rmp_write_page)\n");
    printf("socketfd : %d, connected : %d\n", socketfd, connected);
    if (!connected)
    {
        return;
    }

    rmp::packet send_pkt;
    send_pkt.action = 2;
    memcpy(send_pkt.data, page, PAGE_SIZE);
    send_pkt.size = PAGE_SIZE;
    send_pkt.offset = (u32)offset;
    send_pkt.error = 0;
    send_pkt.hndl = hd;

    rmp::packet return_pkt;

    send(socketfd, &send_pkt, sizeof(rmp::packet), 0);

    printf("send returned (rmp_write_page)\n");

    recv(socketfd, &return_pkt, sizeof(rmp::packet), 0);

    printf("recv returned (rmp_write_page)\n");

    printf("Error (rmp_write_page) : %u\n", return_pkt.error);
}

void rmp_read_page(rmp::handle hd, ul offset, void *page)
{
    // memset(page, 1, 4096);

    printf("entered (rmp_read_page)\n");
    printf("socketfd : %d, connected : %d\n", socketfd, connected);
    if (!connected)
    {
        return;
    }

    rmp::packet send_pkt;
    send_pkt.action = 1;
    send_pkt.offset = 0;
    send_pkt.size = PAGE_SIZE;
    send_pkt.error = 0;
    send_pkt.hndl = hd;

    rmp::packet return_pkt;

    send(socketfd, &send_pkt, sizeof(rmp::packet), 0);

    printf("send returned (rmp_read_page)\n");

    recv(socketfd, &return_pkt, sizeof(rmp::packet), 0);

    printf("recv returned (rmp_read_page)\n");

    printf("Error (rmp_read_page) : %u\n", return_pkt.error);
    printf("Data (rmp_read_page) : %s\n", return_pkt.data);

    memcpy(page, return_pkt.data, PAGE_SIZE);
}

void rmp_pagefault_resovler(void *start_addr, void *faulting_addr, int is_write, void *page)
{
    ul offset = (((ul)faulting_addr & ~(PAGE_SIZE - 1)) - (ul)start_addr);
    rmp::handle hndl = addr_hndl_map->at((ul)start_addr);
    printf("rmp_pagefault_resovler:  ");
    printf("Offset = %ld; ", offset);
    printf("Start Address = %" PRIx64 "; ", start_addr);
    printf("Faulting Address = %" PRIx64 "\n", faulting_addr);
    if (is_write)
    {
        rmp_write_page(hndl, offset, page);
    }
    else
    {
        rmp_read_page(hndl, offset, page);
    }
}

// Setup the args to establish connection
rmp::Client::Client(rmp::config cnf)
{
    this->server_config = cnf;
}

int rmp::Client::rmp_init()
{
    int valread;
    struct sockaddr_in serv_addr;
    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(this->server_config.addr.c_str());
    serv_addr.sin_port = htons(this->server_config.port);

    // // Convert IPv4 and IPv6 addresses from text to binary form
    // if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    // {
    //     printf("\nInvalid address/ Address not supported \n");
    //     return -1;
    // }

    if (connect(socketfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    else
    {
        connected = true;
    }

    if (socketfd == -1)
    {
        printf("Error while establishing connection to server\n");
        return -1;
    }
    uffdman_register_page_resolver(&rmp_pagefault_resovler);

    // Initialize the maps
    addr_hndl_map = new std::unordered_map<ul, rmp::handle>();
    addr_npages_map = new std::unordered_map<ul, ul>();

    return 0;
}

void *rmp::Client::rmp_alloc(u32 npages)
{
    long size = npages * PAGE_SIZE;
    void *new_addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (new_addr == MAP_FAILED)
    {
        printf("Error while allocating memory on client");
        return (char *)MAP_FAILED;
    }

    printf("socketfd : %d, connected : %d\n", socketfd, connected);
    if (!connected)
    {
        return NULL;
    }

    rmp::packet send_pkt;
    send_pkt.action = 0;
    send_pkt.npages = npages;
    send_pkt.size = PAGE_SIZE;

    rmp::packet return_pkt;

    send(socketfd, &send_pkt, sizeof(rmp::packet), 0);

    recv(socketfd, &return_pkt, sizeof(rmp::packet), 0);

    printf("Handle : %d\n", return_pkt.hndl);

    uffdman_register_region(new_addr, npages);

    ul ul_addr = (ul)new_addr;
    addr_hndl_map->insert({ul_addr, return_pkt.hndl});
    addr_npages_map->insert({ul_addr, npages});

    return new_addr;
}

int rmp::Client::rmp_free(rmp::handle hndl)
{
}

void rmp::Client::rmp_read(rmp::handle hd, ul offset, void *page)
{
    rmp_read_page(hd, offset, page);
}

void rmp::Client::rmp_write(rmp::handle hd, ul offset, void *page)
{
    rmp_write_page(hd, offset, page);
}

// Client(config);              // Setup the args to establish connection
// int rmp_init();              // Connect to Server
// rmp::handle rmp_alloc(long); // Allocate n pages on server and returns a handle
// int rmp_free(rmp::handle);   //free pages allocated for the handle