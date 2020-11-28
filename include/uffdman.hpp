#pragma once

void uffdman_init();

void uffdman_destroy();

void uffdman_register_page_resolver(void (*handler)(void *start_addr, void *faulting_addr, int is_write, void *page));

void uffdman_unregister_page_resolver();

int uffdman_register_region(void *addr, unsigned long n_pages);

void uffdman_unregister_region(void *addr);
