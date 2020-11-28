#pragma once

#include <unordered_map>
#include <mutex>
#include "rmp_types.hpp"

namespace rmp
{
	// Request contains fields required to
	// complete a client's request
	typedef struct
	{
		handle hndl;
		u32 npages;
		u32 size;
		u32 offset;
		u32 action;		 // 0 -> map, 1 -> get, 2 -> put, 3 -> free
		char data[4096]; // Data to be read or written
		u32 error;
	} packet;

	typedef struct
	{
		/*
			TODO: Define a commong connection configuration for both guest and host
			
			Example: IPv4, Protocol, Port, etc.
		*/
	} config;

	class Server
	{
	private:
		u32 handle_count = 0;
		config server_config;
		std::unordered_map<rmp::handle, void *> addr_map;
		std::unordered_map<rmp::handle, u32> hndl_n_pages;
		std::mutex handle_lock, addr_map_lock, hndl_n_pages_lock;

	public:
		Server(rmp::config)
		{
		}

		void handle(rmp::packet *);
		rmp::handle alloc(u32, u32);
		int read(rmp::handle, rmp::packet *);
		int write(rmp::handle handle, rmp::packet *packet);
		int free(rmp::handle, u32);

		inline rmp::handle get_new_handle()
		{
			std::lock_guard<std::mutex> guard(this->handle_lock);
			u32 new_handle = handle_count++;
			return new_handle;
		}

		inline void set_addr_map(rmp::handle hndl, void *addr)
		{
			std::lock_guard<std::mutex> guard(this->addr_map_lock);
			this->addr_map[hndl] = addr;
		}

		inline void *get_addr_map(rmp::handle hndl)
		{
			std::lock_guard<std::mutex> guard(this->addr_map_lock);
			return this->addr_map[hndl];
		}

		inline void unset_addr_map(rmp::handle hndl)
		{
			std::lock_guard<std::mutex> guard(this->addr_map_lock);
			this->addr_map.erase(hndl);
		}

		inline void set_hndl_n_pages(rmp::handle hndl, u32 num_pages)
		{
			std::lock_guard<std::mutex> guard(this->hndl_n_pages_lock);
			this->hndl_n_pages[hndl] = num_pages;
		}

		inline u32 get_hndl_n_pages(rmp::handle hndl)
		{
			std::lock_guard<std::mutex> guard(this->hndl_n_pages_lock);
			return this->hndl_n_pages[hndl];
		}

		inline void unset_hndl_n_pages(rmp::handle hndl)
		{
			std::lock_guard<std::mutex> guard(this->hndl_n_pages_lock);
			this->hndl_n_pages.erase(hndl);
		}

		inline bool contains_key(rmp::handle handle)
		{
			return this->addr_map.find(handle) != this->addr_map.end();
		}
	};

	typedef struct
	{
		int client_sock;
		rmp::Server *server;
	} thread_req;
} // namespace rmp