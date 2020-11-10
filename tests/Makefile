SRC=../libsrvcli/*.cpp ../uffdman/*.cpp ../rmp/*.cpp

all: test_srv test_cli
	
test_srv: mkdir
	g++ -g -o bin/test_srv -I../include -I../libsrvcli/include -DDEBUG_PRINTS=1 $(SRC) test_server.cpp -lpthread

test_cli: mkdir
	g++ -g -o bin/test_cli -I../include -I../libsrvcli/include -DDEBUG_PRINTS=1 $(SRC) test_client.cpp -lpthread

mkdir:
	mkdir -p bin

clean:
	rm -rf bin
