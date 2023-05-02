.PHONY: all clean test

mint.exe: driver.cpp mint.cpp
	clang++ -Wall -Werror -Og -g -std=c++20 -o $@ $^

all: mint.exe

clean:
	rm mint.exe *~

test: mint.exe
	./mint.exe data/test-1.txt motifs/m1-test.txt
