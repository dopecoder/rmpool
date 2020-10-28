SRC=../libsrvcli/*.cpp ../uffdman/*.cpp ../rmp/*.cpp
# SRC=../rmp.so

all: preloadlib
	
preloadlib:
	g++ -g -I../include -I../libsrvcli/include $(SRC) -shared -fPIC -o preloadlib.so preloadlib.cpp -ldl -lpthread
	
clean:
	rm -f preloadlib.so
