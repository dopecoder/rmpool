#ifndef RMP_HPP
#define RMP_HPP

#include "ConnectionConfig.hpp"

int rmp_init(ConnectionConfig);
void *rmp_alloc(long);
void rmp_free(void *);

void rmp_server_init(ConnectionConfig);

#endif
