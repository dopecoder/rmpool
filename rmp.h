#ifdef RMP_H
#define RMP_H

namespace rmp {

typedef int rmp_handle;

namespace guest {
// Guest
void rmp_connect(struct config);
char* rmp_alloc(int n_pages); // receives a rmp_handle from host
void rmp_free(rmp_handle hd);

// private guest
void pagefault_handler(void* args);
void rmp_write(rmp_handle hd, int offset, void* data);
void* rmp_read(rmp_handle hd, int offset);
}

namespace host {
// Host
void rmp_host_setup(struct config);
void rmp_host_alloc(int n_pages); // sends a rmp_handle to guest
void rmp_host_free(rmp_handle hd);

// private host
void rmp_host_write(rmp_handle hd, int offset, void* data);
void* rmp_host_read(rmp_handle hd, int offset);
}
}
#endif
