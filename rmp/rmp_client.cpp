#include <unordered_map>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
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

using namespace std;

// static ConnectionConfig server_conf;
static unordered_map<ul, rmp_handle> *addr_hndl_map;
static unordered_map<ul, ul> *addr_npages_map;

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

static void rmp_write_page(rmp_handle hd, ul offset, void *page)
{
	// send page to server with write request
	// srv_handle = hd;
	// srv_offset = offset;
	// srv_page = page;
	// srv_write_page(sockfd);
}

static ul rmp_read_page(rmp_handle hd, ul offset, void *page)
{
	// srv_handle = hd;
	// srv_offset = offset;
	// srv_read_page(sockfd);
	// return srv_page;
	memset(page, 1, 4096);
}

void rmp_pagefault_resovler(void *start_addr, void *faulting_addr, int is_write, void *page)
{
	ul offset = (((ul)faulting_addr & ~(PAGE_SIZE - 1)) - (ul)start_addr);
	rmp_handle hndl = addr_hndl_map->at((ul)start_addr);
	// char *str = "INSIDE rmp_pagefault_resovler\n";
	// write(STDOUT_FILENO, str, strlen(str));
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

int rmp_init(ConnectionConfig conf)
{
	cli = new Client(conf);
	sockfd = cli->connectToServer();
	if (sockfd == -1)
	{
		err("Error while establishing connection to server");
		return -1;
	}
	uffdman_register_page_resolver(&rmp_pagefault_resovler);

	// Initialize the maps
	addr_hndl_map = new unordered_map<ul, rmp_handle>();
	addr_npages_map = new unordered_map<ul, ul>();

	return 0;
}

void *rmp_alloc(long n_pages)
{
	long size = n_pages * PAGE_SIZE;
	void *new_addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if (new_addr == MAP_FAILED)
	{
		err("Error while allocating memory on client");
		return (char *)MAP_FAILED;
	}

	ul ul_addr = (ul)new_addr;

	// request for new allocation of n_pages
	srv_n_pages = n_pages;
	srv_alloc_pages(sockfd);
	if (srv_handle == -1)
	{
		err("Error while allocating memory on server");
		return (char *)MAP_FAILED;
	}

	uffdman_register_region(new_addr, n_pages);

	addr_hndl_map->insert({ul_addr, srv_handle});
	addr_npages_map->insert({ul_addr, n_pages});

	return new_addr;
}

void rmp_free(void *addr)
{
	auto exists = addr_hndl_map->find((ul)addr);
	printf("RMP FREE called\n");

	if (exists != addr_hndl_map->end())
	{
		rmp_handle hndl = addr_hndl_map->at((ul)addr);

		// send free request for hndl to server
		srv_handle = hndl;

		uffdman_unregister_region(addr);

		auto addr_npages_exists = addr_npages_map->find((ul)addr);
		if (addr_npages_exists != addr_npages_map->end())
		{
			// free the invalid address that is stored in the table
			if (munmap(addr, addr_npages_map->at((ul)addr) * PAGE_SIZE) == -1)
				err("Erro while freeing memory on client");

			// remove stored references of address and size
			addr_npages_map->erase((ul)addr);
		}
		addr_hndl_map->erase((ul)addr);
		printf("Calling srv_free_pages\n");
		srv_free_pages(sockfd);
	}
	printf("RMP FREE exiting\n");
}

void rmp_destroy()
{
	delete addr_hndl_map;
	delete addr_npages_map;
}
