#include <unordered_map>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <unistd.h>
#include <iostream>

#include "rmp.hpp"
#include "rmp_types.hpp"
#include "libsrvcli.hpp"

#define PAGE_SIZE sysconf(_SC_PAGE_SIZE)
#define err(msg) 

using namespace std;

static ConnectionConfig server_conf;
static Server* server;

static rmp_handle alloc_count = 0;

static unordered_map<rmp_handle, ul> hndl_addr_map;		
static unordered_map<ul, ul> addr_npages_map; 

static rmp_handle rmp_server_alloc(long n_pages) 
{ 
	long mem_region_size = n_pages * PAGE_SIZE;

	char* remote_mem_region = (char*) mmap(NULL, mem_region_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if(remote_mem_region == MAP_FAILED)
	{
		// TODO Handle error
	}

	++alloc_count;
	ul ul_addr = (ul) remote_mem_region;

	hndl_addr_map[alloc_count] = ul_addr;
	addr_npages_map[ul_addr] = n_pages;

	return alloc_count;
}

static void rmp_server_free(rmp_handle hndl)
{
	ul addr = hndl_addr_map[hndl];
	ul n_pages = addr_npages_map[addr]; 
	long size = n_pages * PAGE_SIZE;
	if (munmap((char*) addr, size) == -1)
	{
		// TODO Handle error
	}
	hndl_addr_map.erase(hndl);
	addr_npages_map.erase(addr);
}

static void rmp_server_write(rmp_handle hndl, long offset, ul page)
{
	char* addr = (char*) hndl_addr_map[hndl];
	*((char*) ((ul)addr + offset)) = (page);
}

static ul rmp_server_read(rmp_handle hndl, long offset)
{
	char* addr = (char*) hndl_addr_map[hndl];
	return *((char*) ((ul)addr + offset)); 
}

static ul srv_n_pages;
static rmp_handle srv_handle;
static ul srv_page, srv_offset;
static enum rmp_req_type req_type;
static void alloc_pages(int sockfd)
{
	recv(sockfd, &srv_n_pages, sizeof(ul), 0);

	srv_handle = rmp_server_alloc(srv_n_pages);

	send(sockfd, &srv_handle, sizeof(rmp_handle), 0);
}

static void read_page(int sockfd)
{
	recv(sockfd, &srv_handle, sizeof(rmp_handle), 0);
	recv(sockfd, &srv_offset, sizeof(ul), 0);
	
	srv_page = rmp_server_read(srv_handle, srv_offset);
	cout << "conn: sending: " << srv_page << endl;
	send(sockfd, &srv_page, sizeof(ul), 0);
}

static void write_page(int sockfd)
{
	recv(sockfd, &srv_handle, sizeof(rmp_handle), 0);
	recv(sockfd, &srv_offset, sizeof(ul), 0);
	recv(sockfd, &srv_page, sizeof(ul), 0);

	cout << "conn: received page: " << srv_page << endl;
	rmp_server_write(srv_handle, srv_offset, srv_page);
}

static void free_pages(int sockfd)
{
	recv(sockfd, &srv_handle, sizeof(rmp_handle), 0);

	rmp_server_free(srv_handle);
}

void on_client_connected(int sockfd)
{
	enum rmp_req_type req_type;
	recv(sockfd, &req_type, sizeof(enum rmp_req_type), 0);

	switch(req_type)
	{
		case ALLOC_PAGES:
			alloc_pages(sockfd);	
			break;
		case READ_PAGE:
			read_page(sockfd);
			break;
		case WRITE_PAGE:
			write_page(sockfd);
			break;
		case FREE_PAGES:
			free_pages(sockfd);
			break;
	}
	close(sockfd);
}

void rmp_server_init(ConnectionConfig conf) 
{
	// restrict incoming/outgoing requests/responses to only given guest
	server_conf = conf; 

	// start a server w/ given config
	server = new Server(&on_client_connected, conf);
	server->start(); // Non blocking call
}
