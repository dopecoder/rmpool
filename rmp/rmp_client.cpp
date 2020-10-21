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

#define PAGE_SIZE sysconf(_SC_PAGE_SIZE)
#define err(msg) 

/*
   Potential TODO:
   - convert client and server to classes
   - useful for multiple instances of servers/clients.
   - Guests can have proxy server objects
 */

using namespace std;

static ConnectionConfig server_conf;
static unordered_map<ul, rmp_handle> addr_hndl_map;		
static unordered_map<ul, ul> addr_npages_map; 

static ul srv_n_pages;
static rmp_handle srv_handle;
static ul srv_page, srv_offset;
static enum rmp_req_type req_type;
static void alloc_pages(int sockfd)
{
	req_type = ALLOC_PAGES;
	send(sockfd, &req_type, sizeof(enum rmp_req_type), 0);

	send(sockfd, &srv_n_pages, sizeof(ul), 0);
	recv(sockfd, &srv_handle, sizeof(rmp_handle), 0);
}

static void read_page(int sockfd)
{
	req_type = READ_PAGE;
	send(sockfd, &req_type, sizeof(enum rmp_req_type), 0);

	send(sockfd, &srv_handle, sizeof(rmp_handle), 0);
	send(sockfd, &srv_offset, sizeof(ul), 0);
	recv(sockfd, &srv_page, sizeof(ul), 0);
	cout << "conn: received page: " << srv_page << endl;
}

static void write_page(int sockfd)
{
	req_type = WRITE_PAGE;
	send(sockfd, &req_type, sizeof(enum rmp_req_type), 0);

	send(sockfd, &srv_handle, sizeof(rmp_handle), 0);
	send(sockfd, &srv_offset, sizeof(ul), 0);
	send(sockfd, &srv_page, sizeof(ul), 0);
}

static void free_pages(int sockfd)
{
	req_type = FREE_PAGES;
	send(sockfd, &req_type, sizeof(enum rmp_req_type), 0);

	send(sockfd, &srv_handle, sizeof(rmp_handle), 0);
}

static void notify_server(void (*func)(int), string err_msg)
{
	Client cli(func, server_conf);
	if(cli.establishConnection() == -1)
		err("Error while allocating pages on server");
}

static void rmp_write_page(rmp_handle hd, ul offset, ul page)
{
	// send page to server with write request
	srv_handle = hd;
	srv_offset = offset;
	srv_page = page;
	notify_server(&write_page, "Error while writing page to server");
}

static ul rmp_read_page(rmp_handle hd, ul offset)
{
	ul page;

	// receive page from server by sending a read request
	srv_handle = hd;
	srv_offset = offset;
	notify_server(&read_page, "Error while reading page from server");
	page = srv_page;

	return page;
}

void rmp_pagefault_resovler(char* start_addr, char* faulting_addr, int is_write, char* page)
{
	ul offset = (((ul) faulting_addr & ~(PAGE_SIZE-1)) - (ul)start_addr);
	//if(offset <= addr_npages_map[(ul) start_addr]) 
	{
		rmp_handle hndl = addr_hndl_map[(ul) start_addr];
		if(is_write) 
		{
			rmp_write_page(hndl, offset, (ul) *page);
		}
		else 
		{
			*page = rmp_read_page(hndl, offset);
		}
	} 
	//else
	{
		// TODO out of memory region
	}
}

void rmp_init(ConnectionConfig conf) 
{
	server_conf = conf;
	uffdman_register_page_resolver(&rmp_pagefault_resovler);
}

char* rmp_alloc(long n_pages) 
{
	long size = n_pages * PAGE_SIZE;
	char* new_addr = (char*) mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0); 

	ul ul_addr = (ul) new_addr;

	// request for new allocation of n_pages
	srv_n_pages = n_pages;
	notify_server(&alloc_pages, "Error while allocating pages on server");

	// receives a rmp_handle from server	
	rmp_handle server_hndl = srv_handle;

	uffdman_register_region(new_addr, n_pages);

	addr_hndl_map[ul_addr] = server_hndl;
	addr_npages_map[ul_addr] = n_pages;

	return new_addr;
}

void rmp_free(char* addr) 
{
	rmp_handle hndl = addr_hndl_map[(ul) addr];
	
	// send free request for hndl to server
	srv_handle = hndl;
	notify_server(&free_pages, "Error while allocating pages on server");

	uffdman_unregister_region(addr);

	// free the invalid address that is stored in the table
	munmap(addr, addr_npages_map[(ul) addr] * PAGE_SIZE);

	// remove stored references of address and size
	addr_hndl_map.erase((ul) addr);
	addr_npages_map.erase((ul) addr);
}
