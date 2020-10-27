#ifndef UFFDMAN_H
#define UFFDMAN_H

void uffdman_register_page_resolver(void (*handler) (char* start_addr, char* faulting_addr, int is_write, char* page));

void uffdman_unregister_page_resolver();

int uffdman_register_region(char *addr, unsigned long n_pages);

void uffdman_unregister_region(char* addr);

#endif
