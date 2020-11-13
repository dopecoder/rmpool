#include <iostream>
#include <string.h>
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
	int sum = 0;

	rmp_init(config);

	char *addr = (char *)rmp_alloc(n);

	// cout << sizeof(char) << endl;
	for (int i = 0; i < 4096 * 5; i++)
	{
		// cout << (char *)(addr + i) << " : " << addr[i] << endl;
		// printf("%lx : %d\n", (char *)(addr + i), addr[i]);
		addr[i] = 1;
	}

	cout << "Finished Writing" << endl;

	// memset(addr, 1, sizeof(char) * 8192);

	// 4096 = sizeof(int) * 1024

	// addr[1024] = 'z';
	for (int i = 0; i < 4096 * 5; i++)
	{
		// cout << (char *)(addr + i) << " : " << addr[i] << endl;
		// printf("%lx : %d\n", (char *)(addr + i), addr[i]);
		sum += addr[i];
	}

	cout << "Sum : " << sum << endl;

	// addr[4800] = 'y';

	// cout << "2: " << addr[4800] << endl;
	// 	/*
	// 	for(int i=0; i<n; i++)
	// 		addr[i*4096] = ('A'+i) % 26;
	// */
	rmp_free((char *)addr);

	return 0;
}
