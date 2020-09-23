

// client
//

static void pf_handler (uint64_t faulting_address)
{
	// is it a special remote address? 
	handle * handle = lookup_in_handle_hashtable(faulting_address);
	if (handle) { 
		// this is a remote region
		//
		if (fault_type == FAULT_READ) {
			read_page(handle);
		} else { // it's  a write
			write_page(handle)
		}

		// OTHERWISE ERROR

	}

}

static char * alloc_remote_mem(size_t size) {
	// use your remote API here
	handle * handle = get_remote_page();
	char * dummy_addr = mmap(....,PROT_NONE,...);
	insert_handle_into_hashtable(dummy_addr, handle);
	return dummy_addr;
}

static void read_page (handle * handle, size_t offset, size_t len)
{
	// use your remote API here
}

static void write_page (handle * handle, size_t offset, size_t len, void * src)
{
	// use your remote API here
}


int main () {
	register_userfaultfd_handler(pf_handler);

	char * remote = alloc_remote_mem(4096); // this needs to give something back which IS NOT MAPPED
	*remote = 0xdeadbeef; // this will cause a page fault, which will vector to pf_handler

	free_remote_mem(remote);
}

