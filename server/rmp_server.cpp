#include <sys/mman.h>
#include <string.h>
#include "rmp_types.hpp"
#include "rmp_server.hpp"

void handle_alloc(rmp::Server *rmp_server, rmp::packet *packet)
{

	rmp::handle new_hndl = rmp_server->alloc(packet->npages, packet->size);
	if (new_hndl == -1)
	{
		packet->error = ALLOC_FAILED;
		return;
	}
	packet->hndl = new_hndl;
}

void handle_read(rmp::Server *rmp_server, rmp::packet *packet)
{

	rmp::handle hndl = packet->hndl;
	if (!(rmp_server->contains_key(hndl)))
	{
		packet->error = INVALID_HANDLE;
		return;
	}

	int read_suc = rmp_server->read(hndl, packet);
	if (read_suc == -1)
	{
		packet->error = READ_FAILED;
		return;
	}
}

void handle_write(rmp::Server *rmp_server, rmp::packet *packet)
{

	rmp::handle hndl = packet->hndl;
	if (!(rmp_server->contains_key(hndl)))
	{
		packet->error = INVALID_HANDLE;
		return;
	}

	int write_suc = rmp_server->write(hndl, packet);
	if (write_suc == -1)
	{
		packet->error = WRITE_FAILED;
		return;
	}
}

void handle_free(rmp::Server *rmp_server, rmp::packet *packet)
{

	rmp::handle hndl = packet->hndl;
	if (!(rmp_server->contains_key(hndl)))
	{
		packet->error = INVALID_HANDLE;
		return;
	}

	int write_suc = rmp_server->free(hndl, packet->size);
	if (write_suc == -1)
	{
		packet->error = FREE_FAILED;
		return;
	}
}

void rmp::Server::handle(rmp::packet *packet)
{
	switch (packet->action)
	{
	case 0:
		handle_alloc(this, packet);
		break;
	case 1:
		handle_read(this, packet);
		break;
	case 2:
		handle_write(this, packet);
		break;
	case 3:
		handle_free(this, packet);
		break;
	default:
		packet->error = UNKNOWN_ACTION;
		break;
	}
}

rmp::handle rmp::Server::alloc(u32 npages, u32 page_size)
{
	ul mem_region_size = npages * page_size;

	void *remote_mem_region = mmap(NULL, mem_region_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if (remote_mem_region == MAP_FAILED)
	{
		return -1;
	}

	for (u32 i = 0; i < npages; i++)
	{
	}
	u32 new_handle = this->get_new_handle();
	this->set_addr_map(new_handle, remote_mem_region);
	this->set_hndl_n_pages(new_handle, npages);
	return new_handle;
}

int rmp::Server::read(rmp::handle handle, rmp::packet *packet)
{
	u32 offset = packet->offset;
	u32 npages =
		this->get_hndl_n_pages(handle);
	ul addr = (ul)this->get_addr_map(handle);
	if (offset >= npages)
	{
		return -1;
	}

	ul addr_with_offset = addr + (packet->size * offset);
	memcpy(packet->data, (void *)addr_with_offset, sizeof(packet->data));
	return 0;
}

int rmp::Server::write(rmp::handle handle, rmp::packet *packet)
{
	u32 offset = packet->offset;
	u32 npages =
		this->get_hndl_n_pages(handle);
	ul addr = (ul)this->get_addr_map(handle);

	if (offset >= npages)
	{
		return -1;
	}

	ul addr_with_offset = addr + (packet->size * offset);
	memcpy((void *)addr_with_offset, packet->data, sizeof(packet->data));
	return 0;
}

int rmp::Server::free(rmp::handle handle, u32 page_size)
{
	if (this->contains_key(handle))
	{
		u32 npages =
			this->get_hndl_n_pages(handle);
		if (npages == 0)
		{
			return -1;
		}

		void *addr = this->get_addr_map(handle);
		long size = npages * page_size;
		if (munmap(addr, size) == -1)
		{
			return -1;
		}

		this->unset_addr_map(handle);
		this->unset_hndl_n_pages(handle);
		return 0;
	}
	return -1;
}