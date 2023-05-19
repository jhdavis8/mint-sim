# Mint Simulator

This is a very simple simulator of the Mint domain-specific architecture that I created for CMSC818J in Spring 2023.

The original Mint paper by Talati et al. is available at [https://ieeexplore.ieee.org/document/9923899](https://ieeexplore.ieee.org/document/9923899).

SNAP datasets available at [Stanford SNAP webpage.](http://snap.stanford.edu/data/index.html#:~:text=with%20traffic%20information.-,Temporal%20networks,-Name)

## Using MintSim

Type `make` to build MintSim. The Makefile is very simple, it just assumes you have GCC installed. Support for C++20 is required.

Type `make test` to run a very simple accuracy test of the simulator. It should return one motif match found.

While all the motifs needed for the report results are included, the datasets are not, and must be downloaded from the SNAP link above and copied into the data directory.

Once this is done, you can run the `run-experiments.sh` script to reproduce all of the experimental results from the report. The output files will be placed under results, which currently contains the resulst from my runs.
