// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "rmp_server.hpp"

#define PORT 6767

int server_fd, client_sock, *new_sock;
rmp::Server *server;

void *connection_handler(void *);
void init()
{
	server = new rmp::Server(rmp::config{});
}

int main(int argc, char const *argv[])
{
	int opt = 1;
	struct sockaddr_in address;
	int addrlen = sizeof(address);

	if (argc != 3)
	{
		std::cout << "Invalid args!" << std::endl;
		std::cout << "Usage: ./server <ip> <port>" << std::endl;
		return 1;
	}

	std::cout << argv[1] << std::endl;
	std::cout << atoi(argv[2]) << std::endl;

	init();

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
	address.sin_addr.s_addr = inet_addr(argv[1]);
	address.sin_port = htons(atoi(argv[2]));

	//	Forcefully attaching socket to the port 8080
	if (bind(server_fd, (struct sockaddr *)&address,
			 sizeof(address)) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	if (listen(server_fd, 3) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	while ((client_sock = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)))
	{
		puts("Connection accepted");

		pthread_t sniffer_thread;
		rmp::thread_req *thread_info = new rmp::thread_req;
		thread_info->client_sock = client_sock;
		thread_info->server = server;

		if (pthread_create(&sniffer_thread, NULL, connection_handler, (void *)thread_info) < 0)
		{
			perror("could not create thread");
			return 1;
		}

		pthread_join(sniffer_thread, NULL);
		puts("Thread assigned");
	}

	if (client_sock < 0)
	{
		perror("Client connection accept failed!");
		return 1;
	}

	return 0;
}

void *connection_handler(void *thread_info)
{
	int read_size;
	rmp::packet *req = new rmp::packet;
	rmp::thread_req *info = (rmp::thread_req *)thread_info;
	int sock = info->client_sock;

	while ((read_size = recv(sock, req, sizeof(rmp::packet), 0)) > 0)
	{
		printf("Recieved Bytes : %ld\n", read_size);
		printf("action : %d\n", req->action);
		if (read_size == sizeof(rmp::packet))
		{

			info->server->handle(req);
			printf("Handle Returned For Action : %d\n", req->action);
			send(sock, req, sizeof(rmp::packet), 0);
			printf("Finished writing back results : %d\n", req->action);
		}
		else
		{
			break;
		}
		printf("Completed Action : %d\n", req->action);
	}

	if (read_size == 0)
	{
		puts("Client disconnected");
		fflush(stdout);
	}
	else if (read_size == -1)
	{
		perror("recv failed");
	}

	free(thread_info);
	return 0;
}