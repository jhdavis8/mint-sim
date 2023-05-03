#!/bin/bash

# Usage: ./run-case.sh data motif missrate memo thresh

export CACHE_MISS $3
export USE_MEMO $4
export MEMO_THRESH $5
make clean
make
./mint.exe data/$1 motif/$2 |& tee $1_$2_$3_$4_$5.txt
