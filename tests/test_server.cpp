#include <iostream>

#include "rmp.hpp"
#include "ConnectionConfig.hpp"

using namespace std;

int main(int argc, char** argv)
{
	if(argc != 3)
	{
		cout << "Invalid args!" << endl;
		cout << "Usage: ./test_srv <ip> <port>" << endl;
		return 1;
	}

	ConnectionConfig config(argv[1], argv[2], "");
	cout << config.getIP() << endl;
	cout << config.getPort() << endl;

	rmp_server_init(config);		
	
	while(true);
	
	return 0;
}
