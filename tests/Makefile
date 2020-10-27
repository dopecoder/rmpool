SRC=../libsrvcli/*.cpp ../uffdman/*.cpp ../rmp/*.cpp
# SRC=-L../rmp.so -L../libsrvcli/libsrvcli.so 

all: test_srv test_cli
	
test_srv:
	g++ -g -o test_srv -I../include -I../libsrvcli/include -DDEBUG_PRINTS=1 $(SRC) test_server.cpp -lpthread
test_cli:
	g++ -g -o test_cli -I../include -I../libsrvcli/include -DDEBUG_PRINTS=1 $(SRC) test_client.cpp -lpthread
clean:
	rm -f test_cli test_srv
