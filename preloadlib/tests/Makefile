
all: malloc_test calloc_test realloc_test

malloc_test: mkdir
	gcc -g malloc_test.c -o ./bin/malloc_test

calloc_test: mkdir
	gcc -g calloc_test.c -o ./bin/calloc_test

realloc_test: mkdir
	gcc -g realloc_test.c -o ./bin/realloc_test

mkdir:
	mkdir -p bin

clean:
	rm -rf bin
	