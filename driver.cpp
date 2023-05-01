// Driver for the Mint simulator

#include <iostream>
#include <ifstream>
#include "mint.hpp"

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Error: must provide temporal graph data file." << std::endl;
  } else if (argc > 2) {
    std::cerr << "Error: unrecognized argument(s)." << std::endl;
  }
  std::vector<Edge> edgeList;
  std::ifstream dataFile(argv[1]);
  if (dataFile.is_open()) {
    dataFile.close(); 
  } else {
    std::cerr << "Error: could not open provided graph data file." << std::endl;
    return 1;
  }
  return 0;
}
