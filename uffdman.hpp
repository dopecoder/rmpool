#ifndef UFFDMAN_H
#define UFFDMAN_H

void register_uffd_page_resolver(void (*handler) (char* start_addr, char* faulting_addr, long page_size, int is_write, char** page));

void unregister_uffd_page_resolver();

void register_uffd_region(char *addr, unsigned long n_pages);

void unregister_uffd_regoin(char* addr);

#endif
