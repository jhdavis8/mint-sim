.PHONY: all clean test

mint.exe: driver.cpp mint.cpp mint.hpp Makefile
	g++ -fopenmp -Wall -O3 -std=c++20 -o $@ driver.cpp mint.cpp

all: mint.exe

clean:
	rm mint.exe *~

test: mint.exe
	./mint.exe data/test-1.txt motifs/m1-test.txt
