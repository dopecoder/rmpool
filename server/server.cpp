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
	server = new rmp::Server(rmp::config());
}

/*
 * Reads n bytes from sd into where p points to.
 *
 * returns 0 on succes or -1 on error.
 *
 * Note: 
 * The function's name is inspired by and dedicated to "W. Richard Stevens" (RIP).
 */
int readn(int sd, void *p, size_t n)
{
	size_t bytes_to_read = n;
	size_t bytes_read = 0;

	while (bytes_to_read > bytes_read)
	{
		ssize_t result = read(sd, p + bytes_read, bytes_to_read);
		if (-1 == result)
		{
			if ((EAGAIN == errno) || (EWOULDBLOCK == errno))
			{
				continue;
			}

#ifdef DEBUG
			{
				int errno_save = errno;
				perror("read() failed");
				errno = errno_save;
			}
#endif

			break;
		}
		else if (0 == result)
		{
#ifdef DEBUG
			{
				int errno_save = errno;
				fprintf(stderr, "%s: Connection closed by peer.", __FUNCTION__);
				errno = errno_save;
			}
#endif

			break;
		}

		bytes_to_read -= result;
		bytes_read += result;
	}

	return (bytes_read < bytes_to_read) ? -1 : 0;
}

int writen(const int sd, void *b, const size_t s, const int retry_on_interrupt)
{
	size_t n = s;
	while (0 < n)
	{
		ssize_t result = write(sd, b, n);
		if (-1 == result)
		{
			if ((retry_on_interrupt && (errno == EINTR)) || (errno == EWOULDBLOCK) || (errno == EAGAIN))
			{
				continue;
			}
			else
			{
				break;
			}
		}

		n -= result;
		b += result;
	}

	return (0 < n) ? -1 : 0;
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

	// while (readn(sock, req, sizeof(*req)) != -1)
	while ((read_size = recv(sock, req, sizeof(*req), 0)) > 0)
	{
		// printf("Recieved Bytes : %ld\n", read_size);
		// printf("action : %d\n", req->action);
		// printf("handle : %d\n", req->hndl);
		// printf("error : %d\n", req->error);
		// if (read_size == sizeof(rmp::packet))
		// {
		// if (req->action == 0 || req->action == 1 || req->action == 2 || req->action == 3)
		// {
		info->server->handle(req);
		// printf("Handle Returned For Action : %d\n", req->action);

		size_t sent_bytes = send(sock, req, sizeof(rmp::packet), 0);
		// sleep(0.1);
		// printf("Sent Bytes : %d\n", sent_bytes);

		// int retry_on_interrupt = 1;
		// int result = writen(sock, req, sizeof(*req), retry_on_interrupt);
		// if (-1 == result)
		// {
		// 	perror("writen()");
		// }

		// printf("Finished writing back results : %d\n", req->action);
		if (req->action == 0)
		{
			printf("Allocated %d pages with handle : %d\n", req->npages, req->hndl);
		}
		else if (req->action == 1)
		{
			printf("Client Reading page : %d for handle : %d\n", req->offset, req->hndl);
		}
		else if (req->action == 2)
		{
			printf("Client Writing page : %d for handle : %d\n", req->offset, req->hndl);
		}
		else if (req->action == 3)
		{
			printf("Client Freeing handle : %d\n", req->hndl);
		}
		// }
		// }
		// else
		// {
		// 	break;
		// }
	}

	printf("READ SIZE IN THE END : %d\n", read_size);

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