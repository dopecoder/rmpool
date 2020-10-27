CC:=g++ 
CFLAGS:=-fPIC -c
LDFLAGS:=-Iinclude -Ilibsrvcli/include -lpthread 

ifdef $(DEBUG)
CFLAGS += -DDEBUG_PRINTS=$(DEBUG)
endif

all: libsrvcli/libsrvcli.so rmp_client.o rmp_server.o
	$(CC) -shared -o rmp.so libsrvcli/libsrvcli.so rmp_server.o rmp_client.o $(LDFLAGS)

libsrvcli/libsrvcli.so: 
	$(MAKE) -C libsrvcli/ 

rmp_server.o: rmp/rmp_server.cpp
	$(CC) $(CFLAGS) rmp/rmp_server.cpp -o rmp_server.o $(LDFLAGS)

rmp_client.o: rmp/rmp_client.cpp uffdman.o
	$(CC) $(CFLAGS) rmp/rmp_client.cpp -o rmp_client.o $(LDFLAGS) -luffdman.o

uffdman.o: uffdman/uffdman.cpp
	$(CC) $(CFLAGS) uffdman/uffdman.cpp -o uffdman.o $(LDFLAGS) 

clean: 
	$(MAKE) -C libsrvcli/ clean
	rm -f rmp.so rmp_server.o rmp_client.o uffdman.o

.PHONY: libsrvcli/libsrvcli.so
