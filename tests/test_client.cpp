#include <iostream>

#include "rmp.hpp"

using namespace std;

int main(int argc, char **argv)
{
	if (argc != 4)
	{
		cout << "Invalid args!" << endl;
		cout << "Usage: ./test_cli <ip> <port> <n_pages>" << endl;
		return 1;
	}

	ConnectionConfig config(
		(argv[1]),
		(argv[2]),
		"");

	int n = atoi(argv[3]);

	rmp_init(config);

	char *addr = rmp_alloc(n);

	addr[1024] = 'z';

	cout << "1: " << addr[1024] << endl;

	addr[4800] = 'y';

	cout << "2: " << addr[4800] << endl;
	// 	/*
	// 	for(int i=0; i<n; i++)
	// 		addr[i*4096] = ('A'+i) % 26;
	// */
	rmp_free(addr);

	return 0;
}
