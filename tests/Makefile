SRC=../libsrvcli/*.cpp ../uffd/*.cpp ../rmp/*.cpp

all: test_srv test_cli
	
test_srv:
	g++ -g -o test_srv -I../include -I../libsrvcli/include $(SRC) test_server.cpp -lpthread
test_cli:
	g++ -g -o test_cli -I../include -I../libsrvcli/include $(SRC) test_client.cpp -lpthread
	
