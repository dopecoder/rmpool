CC=g++
CFLAGS=

all: client server
	
client: bin_dir
	$(CC) client/*.cpp -o bin/client

server: bin_dir
	$(CC) server/*.cpp -o bin/server

bin_dir:
	mkdir -p bin

clean:
	rm -r bin
	
