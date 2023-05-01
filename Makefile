.PHONY: all clean

mint.exe: driver.cpp mint.cpp
	clang++ -Og -o $@ $^

all: mint.exe

clean:
	rm mint.exe
