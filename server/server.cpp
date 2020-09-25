// Server side C/C++ program to demonstrate Socket programming 
#include <unistd.h> 
#include <fcntl.h>
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h>
#include <sys/mman.h>
#define PORT 8080 

int create_mmap(size_t sz) {
	int handle = 12341;
	char* addr = (char*)mmap(NULL, sz, PROT_WRITE, MAP_PRIVATE, MAP_ANONYMOUS, 0);
	hasbtable_insert(addr, handle);
	printf("mmap address: %s\n", addr);
	return fd;
}

int main(int argc, char const *argv[]) 
{ 
	int server_fd, new_socket, valread; 
	struct sockaddr_in address; 
	int opt = 1; 
	int addrlen = sizeof(address); 
	char buffer[1024] = {0}; 
	const char *hello = "Hello from server"; 

	//	Creating socket file descriptor 
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 

	//	Forcefully attaching socket to the port 8080 
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
				&opt, sizeof(opt))) 
	{ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( PORT ); 

	//	Forcefully attaching socket to the port 8080 
	if (bind(server_fd, (struct sockaddr *)&address, 
				sizeof(address))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	if (listen(server_fd, 3) < 0) 
	{ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	} 
	if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
					(socklen_t*)&addrlen))<0) 
	{ 
		perror("accept"); 
		exit(EXIT_FAILURE); 
	} 

	valread = read( new_socket, buffer, 1024);
	int page_sz = atoi(buffer);
	printf("Page size received: %d\n", page_sz);

	int fd = create_mmap(page_sz);
	char fd_str[20];
	sprintf(fd_str, "%d", fd);
	send(new_socket, fd_str, strlen(fd_str), 0);
	printf("File descriptor sent: %s\n", fd_str);
	close(fd);
	return 0;	
} 
