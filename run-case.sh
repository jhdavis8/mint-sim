#!/bin/bash

# Usage: ./run-case.sh data motif missrate memo thresh

make clean
g++ -DCACHE_MISS=$3 -DUSE_MEMO=$4 -DMEMO_THRESH=$5 -fopenmp -Wall -O3 -std=c++20 -o mint.exe driver.cpp mint.cpp
./mint.exe data/$1 motifs/$2 |& tee $1_$2_$3_$4_$5.txt
