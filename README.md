# Mint Simulator

This is a very simple simulator of the Mint domain-specific architecture that I created for CMSC818J in Spring 2023.

The original Mint paper by Talati et al. is available at [https://ieeexplore.ieee.org/document/9923899](https://ieeexplore.ieee.org/document/9923899).

SNAP datasets available at [Stanford SNAP webpage.](http://snap.stanford.edu/data/index.html#:~:text=with%20traffic%20information.-,Temporal%20networks,-Name)

## Using MintSim

Type `make` to build MintSim. The Makefile is very simple, it just assumes you have GCC installed. Support for C++20 is required.

Type `make test` to run a very simple accuracy test of the simulator. It should return one motif match found.

While all the motifs needed for the report results are included, the datasets are not, and must be downloaded from the SNAP link above and copied into the data directory.

Once this is done, you can run the `run-experiments.sh` script to reproduce all of the experimental results from the report. The output files will be placed in the repo root. `results` currently contains the results from my runs.

`mint.exe` accepts strictly two command line arguments, the path to the dataset file to search over and the path to the motif file to search for.

## MintSim Organization

There are three code files in MintSim:

1. `driver.cpp` contains the driver code which reads command line arguments, opens the files, and begins the simulation.
2. `mint.hpp` is a header file that describes the structure of the code. At the top of the file you can modify defaults for various global macros that control things like memoization threshold and cycle latency assumptions. The memoization code is also in this file.
3. `mint.cpp` contains everything else, i.e., the implementation of all the component simulations for the Mint architecture.

`run-case.sh` and `run-experiments.sh` are a helper script and runner script for reproducing results easily.

The three folders in the repo are:

1. `data`: input graphs to search over
2. `motifs`: temporal motifs to search for
3. `results`: storage for run outputs
