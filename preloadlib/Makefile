SRC=../uffdman/uffdman.cpp ../client/rmp_client.cpp
# SRC=../rmp.so

all: preloadlib preload-tests
	
preloadlib: mkdir
	g++ -g -I ../include -I ../client/ $(SRC) -shared -fPIC -o ./bin/preloadlib.so preloadlib.cpp -ldl -lpthread

preload-tests:
	$(MAKE) -C tests/ all

mkdir:
	mkdir -p bin
	
clean: clean-tests
	rm -rf bin

clean-tests:
	$(MAKE) -C tests/ clean

