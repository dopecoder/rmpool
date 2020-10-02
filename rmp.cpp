#include <rmp>
#include <unordered_map>
#include <sys/mman.h>
#include <uffdman>
#include <unistd.h>

/*
	Potential TODO:
	- convert guest and host to classes
		- useful for multiple instances of hosts/guests.
		- Guests can have proxy host objects
*/
namespace rmp
{
	namespace guest
	{
		static struct config host_conf;

		static unordered_map<ul, rmp_handle> addr_hndl_map;		
		static unordered_map<ul, ul> addr_npages_map; 

		void rmp_init(struct config conf) {
			host_conf = conf;
			// TODO connect to host
			register_uffd_page_resolver(&rmp_pagefault_resolver);
		}

		char* rmp_alloc(ul n_pages) 
		{
			// new invalid address at guest side to cause pagefault upon it's access
			char* new_invalid_addr = mmap(NULL, 0, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0); 
			
			ul ul_addr = (ul) new_invalid_addr;

			// TODO request for new allocation of n_pages

			// TODO receives a rmp_handle from host	
			rmp_handle server_hndl;

			register_uffd_region(new_invalid_addr, n_pages);

			addr_hndl_map[ul_addr] = server_hndl;
			addr_npages_map[ul_addr] = n_pages;

			return new_invalid_addr;
		}

		void rmp_free(char* addr) 
		{
			rmp_handle hndl = addr_hndl_map[(ul) addr];
			// TODO send free request for hndl to server
			// TODO free the invalid address that is stored in the table
			
			// remove stored references of address and size
			addr_hndl_map.erase((ul) addr);
			addr_npages_map.erase((ul) addr);
		}

		// private guest
		void rmp_pagefault_resovler(char* start_addr, char* faulting_addr, long page_size, int is_write, char** page) 
		{
			ul offset = (ul) ((faulting_addr - start_addr) / page_size);
			if(offset <= addr_npages_map[start_addr]) {
				rmp_handle hndl = addr_hndl_map[start_addr];
				if(is_write) 
				{
					rmp_write(hndl, offset, *page);
				}
				else 
				{
					*page = rmp_read(hndl, offset);
				}
			} else {
				// TODO out of memory region
			}
		}
		
		static void rmp_write_page(rmp_handle hd, ul offset, char* page)
		{
			// TODO send page to server with write request
		}
		
		static char* rmp_read_page(rmp_handle hd, ul offset)
		{
			char* page;
			// TODO receive page from server by sending a read request
			return page;
		}
	}

	namespace host 
	{
		static struct confing guest_conf;
		static rmp_hanle alloc_count = 0;
		static long page_size = -1;

		static unordered_map<rmp_handle, ul> hndl_addr_map;		
		static unordered_map<ul, ul> addr_npages_map; 
		
		// Host
		void rmp_host_init(struct config conf) 
		{
			// restrict incoming/outgoing requests/responses to only given guest
			guest_conf = conf; 

			page_size = sysconf(_SC_PAGE_SIZE);

			// TODO start a host w/ given config

			/* 
				TODO hook incoming requests w/ functions below 
			*/

			// TODO print host config using rmp_host_config to console
			
		}

		struct config rmp_host_config() {
			struct config host_conf;
			// TODO Fill host configuration
			return host_conf;
		}

		static rmp_handle rmp_host_alloc(long n_pages) 
		{ 
			long mem_region_size = n_pages * page_size;

			char* remote_mem_region = mmap(NULL, mem_region_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
			
			if(remote_mem_region == MAP_FAILED)
			{
				// TODO Handle error
			}

			++alloc_count;
			ul ul_addr = (ul) remote_mem_region;
			
			hndl_addr_map[alloc_count] = ul_addr;
			addr_npages[ul_addr] = np_pages;
			
			return alloc_count;
		}

		static void rmp_host_free(rmp_handle hndl)
		{
			ul addr = hdnl_addr_map[hndl];
			ul n_pages = addr_npages_map[addr]; 
			long size = n_pages * page_size;
			if (munmap(addr, size) == -1)
			{
				// TODO Handle error
			}
			hdnl_addr_map.erase(hndl);
			addr_npages_map.erase(addr);
		}

		static void rmp_host_write(rmp_handle hndl, long offset, char* page)
		{
			char* addr = (char*) hndl_addr_map[hndl];
			*(addr + (char*) offset) = *(page);
		}

		static char* rmp_host_read(rmp_handle hndl, long offset)
		{
			char* addr = (char*) hndl_addr_map[hndl];
			return (addr + (char*) offset);
		}
	}
}
