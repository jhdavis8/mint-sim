.PHONY: all clean

mint.exe: driver.cpp mint.cpp
	clang++ -Og -std=c++20 -o $@ $^

all: mint.exe

clean:
	rm mint.exe *~
