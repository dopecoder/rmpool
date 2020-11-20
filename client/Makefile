CXX=g++
OTH_INC_DIR = ../include
CXXFLAGS=-c -I $(OTH_INC_DIR)  # Ugly!
DEPS = $(OTH_INC_DIR)/uffdman.hpp
SRC=../uffdman/*.cpp

all: client

client: uffdman.o
	$(CXX) -o bin/client -pthread bin/uffdman.o bin/rmp_client.o bin/client.o 

#rmp_client.cpp ../uffdman/uffdman.cpp client.cpp


# rmp_test: rmp_test.o
# 	$(CXX) -o bin/rmp_test -pthread bin/rmp_test.o bin/rmp_server.o

# rmp_test.o: rmp_server.o
# 	$(CXX) -c rmp_test.cpp -o bin/rmp_test.o

uffdman.o: rmp_client.o
	$(CXX) $(CXXFLAGS) ../uffdman/uffdman.cpp -o bin/uffdman.o

rmp_client.o: client.o
	$(CXX) $(CXXFLAGS) rmp_client.cpp -o bin/rmp_client.o 

client.o: mkdir
	$(CXX) -c client.cpp -o bin/client.o

mkdir:
	mkdir -p bin

clean:
	rm -rf bin
