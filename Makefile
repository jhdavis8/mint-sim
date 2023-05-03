.PHONY: all clean test

mint.exe: driver.cpp mint.cpp
#clang++ -fopenmp -Wall -O3 -std=c++20 -o $@ $^
	g++ -fopenmp -Wall -O3 -std=c++20 -o $@ $^

all: mint.exe

clean:
	rm mint.exe *~

test: mint.exe
	./mint.exe data/test-1.txt motifs/m1-test.txt
