#include <unordered_map>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <unistd.h>
#include <iostream>

#include "rmp.hpp"
#include "rmp_types.hpp"
#include "libsrvcli.hpp"
#include "uffdman.hpp"

#ifndef DEBUG_PRINTS
#define DEBUG_PRINTS 0
#endif

#if DEBUG_PRINTS
#define err(msg) perror(msg)
#define cout(exp) cout << exp;
#else
#define err(msg)
#define cout(exp)
#endif

#define PAGE_SIZE sysconf(_SC_PAGE_SIZE)

/*
   Potential TODO:
   - convert client and server to classes
   - useful for multiple instances of servers/clients.
   - Guests can have proxy server objects
 */

// extern void print(const char *);

using namespace std;

// static ConnectionConfig server_conf;
static unordered_map<ul, rmp_handle> addr_hndl_map;
static unordered_map<ul, ul> addr_npages_map;

Client *cli;
int sockfd = -1;

static ul srv_n_pages;
static rmp_handle srv_handle;
static ul srv_page, srv_offset;
static enum rmp_req_type req_type;
static void srv_alloc_pages(int sockfd)
{
	req_type = ALLOC_PAGES;
	send(sockfd, &req_type, sizeof(enum rmp_req_type), 0);
	cout("Sending request type: " << req_type << endl);
	send(sockfd, &srv_n_pages, sizeof(ul), 0);
	cout("Sending no. of pages: " << srv_n_pages << endl);

	recv(sockfd, &srv_handle, sizeof(rmp_handle), 0);
	cout("Received server handle: " << srv_handle << endl);
}

static void srv_read_page(int sockfd)
{
	req_type = READ_PAGE;
	send(sockfd, &req_type, sizeof(enum rmp_req_type), 0);
	cout("Sending request type: " << req_type << endl);

	send(sockfd, &srv_handle, sizeof(rmp_handle), 0);
	cout("Sending server handle: " << srv_handle << endl);
	send(sockfd, &srv_offset, sizeof(ul), 0);
	cout("Sending offset: " << srv_offset << endl);
	recv(sockfd, &srv_page, sizeof(ul), 0);
	cout("Received page: " << srv_page << endl);
}

static void srv_write_page(int sockfd)
{
	req_type = WRITE_PAGE;
	send(sockfd, &req_type, sizeof(enum rmp_req_type), 0);
	cout("Sending request type: " << req_type << endl);

	send(sockfd, &srv_handle, sizeof(rmp_handle), 0);
	cout("Sending server handle: " << srv_handle << endl);
	send(sockfd, &srv_offset, sizeof(ul), 0);
	cout("Sending offset: " << srv_offset << endl);
	send(sockfd, &srv_page, sizeof(ul), 0);
	cout("Sending page: " << srv_page << endl);
}

static void srv_free_pages(int sockfd)
{
	req_type = FREE_PAGES;
	send(sockfd, &req_type, sizeof(enum rmp_req_type), 0);
	cout("Sending request type: " << req_type << endl);

	send(sockfd, &srv_handle, sizeof(rmp_handle), 0);
	cout("Sending server handle: " << srv_handle << endl);
}

static void rmp_write_page(rmp_handle hd, ul offset, ul page)
{
	// send page to server with write request
	srv_handle = hd;
	srv_offset = offset;
	srv_page = page;
	srv_write_page(sockfd);
}

static ul rmp_read_page(rmp_handle hd, ul offset)
{
	srv_handle = hd;
	srv_offset = offset;
	srv_read_page(sockfd);
	return srv_page;
}

void rmp_pagefault_resovler(char *start_addr, char *faulting_addr, int is_write, char *page)
{
	ul offset = (((ul)faulting_addr & ~(PAGE_SIZE - 1)) - (ul)start_addr);
	rmp_handle hndl = addr_hndl_map[(ul)start_addr];
	if (is_write)
	{
		rmp_write_page(hndl, offset, (ul)*page);
	}
	else
	{
		*page = rmp_read_page(hndl, offset);
	}
}

int rmp_init(ConnectionConfig conf)
{
	printf("RMP_INIT\n");
	// server_conf = conf;
	cout("Before New Client" << endl);
	cli = new Client(conf);
	cout("After New Client" << endl);
	sockfd = cli->connectToServer();
	if (sockfd == -1)
	{
		err("Error while establishing connection to server");
		return -1;
	}
	cout("Client Started" << endl);
	uffdman_register_page_resolver(&rmp_pagefault_resovler);
	return 0;
}

char *rmp_alloc(long n_pages)
{
	printf("1\n");
	long size = n_pages * PAGE_SIZE;
	printf("2\n");
	char *new_addr = (char *)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	printf("3\n");

	if (new_addr == MAP_FAILED)
	{
		printf("13\n");
		err("Error while allocating memory on client");
		return (char *)MAP_FAILED;
	}
	printf("5\n");

	ul ul_addr = (ul)new_addr;

	printf("6\n");
	// request for new allocation of n_pages
	srv_n_pages = n_pages;
	printf("7\n");
	srv_alloc_pages(sockfd);
	printf("8\n");
	if (srv_handle == -1)
	{
		printf("14\n");
		err("Error while allocating memory on server");
		return (char *)MAP_FAILED;
	}

	printf("9\n");
	uffdman_register_region(new_addr, n_pages);

	printf("10\n");
	addr_hndl_map.reserve(1024);
	addr_hndl_map[ul_addr] = srv_handle;
	printf("11\n");
	addr_npages_map.reserve(1024);
	addr_npages_map[ul_addr] = n_pages;
	printf("12\n");

	return new_addr;
}

void rmp_free(char *addr)
{
	rmp_handle hndl = addr_hndl_map[(ul)addr];

	// send free request for hndl to server
	srv_handle = hndl;

	uffdman_unregister_region(addr);

	// free the invalid address that is stored in the table
	if (munmap(addr, addr_npages_map[(ul)addr] * PAGE_SIZE) == -1)
		err("Erro while freeing memory on client");

	// remove stored references of address and size
	addr_hndl_map.erase((ul)addr);
	addr_npages_map.erase((ul)addr);
	srv_free_pages(sockfd);
}
