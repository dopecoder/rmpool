CXX=g++
INC_DIR = ../include
CXXFLAGS=-c -Wall -I$(INC_DIR)  # Ugly!
DEPS = $(INC_DIR)/rmp.hpp

all: server

server: server.o
	$(CXX) -o bin/server -pthread bin/server.o bin/rmp_server.o

server.o: rmp_test
	$(CXX) $(CXXFLAGS) -c server.cpp -o bin/server.o

rmp_test: rmp_test.o
	$(CXX) -o bin/rmp_test -pthread bin/rmp_test.o bin/rmp_server.o

rmp_test.o: rmp_server.o
	$(CXX) $(CXXFLAGS) rmp_test.cpp -o bin/rmp_test.o

rmp_server.o: mkdir
	$(CXX) $(CXXFLAGS) rmp_server.cpp -o bin/rmp_server.o



mkdir:
	mkdir -p bin

clean:
	rm -rf bin
