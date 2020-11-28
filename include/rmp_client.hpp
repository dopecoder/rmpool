#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <cmath>
#include "rmp_types.hpp"

namespace rmp
{

    // Request contains fields required to
    // complete a client's request
    typedef struct
    {
        rmp::handle hndl;
        u32 npages;
        u32 size;
        u32 offset;
        u32 action;      // 0 -> map, 1 -> get, 2 -> put, 3 -> free
        char data[4096]; // Data to be read or written
        u32 error;
    } packet;

    typedef struct
    {
        /*
			TODO: Define a commong connection configuration for both guest and host
			
			Example: IPv4, Protocol, Port, etc.
		*/
        std::string addr;
        ul port;
    } config;

    class Client
    {
    private:
        u32 handle_count = 0;
        config server_config;
        std::vector<rmp::handle> handles;

    public:
        Client(config);                                        // Setup the args to establish connection
        int rmp_init();                                        // Connect to Server
        void *rmp_alloc(u32);                                  // Allocate n pages on server and returns a handle
        int rmp_free(rmp::handle);                             //free pages allocated for the handle
        void rmp_read(rmp::handle hd, ul offset, void *page);  // Read a page offset from a specific handle
        void rmp_write(rmp::handle hd, ul offset, void *page); // Write a page at offset for a specific handle

        // inline int get_offset(ul addr, ul fault_addr, u32 page_size)
        // {
        //     for (int i = 0; i < handles.size(); i++)
        //     {
        //         rmp::handle hndl = this->handles[i];
        //         u32 npages = this->addr_npages_map[i];
        //         ul start_addr = this->addr_hndl_map[hndl];
        //         ul end_addr = start_addr + (npages * page_size);
        //         if (fault_addr >= start_addr && fault_addr <= end_addr)
        //         {
        //             u32 offset = std::floor((fault_addr - start_addr) / page_size);
        //             return offset;
        //         }
        //     }
        //     return -1;
        // }
    };
} // namespace rmp