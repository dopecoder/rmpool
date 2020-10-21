#ifndef RMP_HPP
#define RMP_HPP

#include "ConnectionConfig.hpp"

void rmp_init(ConnectionConfig);
char* rmp_alloc(long); 
void rmp_free(char*);

void rmp_server_init(ConnectionConfig);

#endif
