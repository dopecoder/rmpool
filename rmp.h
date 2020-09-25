#ifdef RMP_H
#define RMP_H

#include <netinet.h>

typedef struct {
	int id;
	struct sockaddr_in sock_addr;
} rmp_host;

void rmp_connect();
void rmp_alloc();
void rmp_free();
void rmp_write();
void rmp_read();

void rmp_host_setup();

#endif
