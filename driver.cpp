// Driver for the Mint simulator

#include <iostream>
#include <fstream>
#include <string>
#include "mint.hpp"

size_t split(const std::string &txt, std::vector<std::string> &strs, char ch) {
    size_t pos = txt.find(ch);
    size_t initialPos = 0;
    strs.clear();

    // Decompose statement
    while (pos != std::string::npos) {
        strs.push_back(txt.substr(initialPos, pos - initialPos));
        initialPos = pos + 1;

        pos = txt.find(ch, initialPos);
    }

    // Add the last one
    strs.push_back(txt.substr(initialPos,
                              std::min(pos, txt.size()) - initialPos + 1));

    return strs.size();
}

int loadFiles(int argc, char** argv, std::vector<Edge>& edgeList,
              std::vector<Edge>& motif) {
  if (argc < 3) {
    std::cerr <<
        "Error: must provide temporal graph and target motif data files." <<
        std::endl;
    return 1;
  } else if (argc > 3) {
    std::cerr << "Error: unrecognized argument(s)." << std::endl;
    return 1;
  }
  std::ifstream dataFileG(argv[1]);
  std::string line;
  if (dataFileG.is_open()) {
    while (std::getline(dataFileG, line)) {
      std::vector<std::string> v;
      Edge e;
      split(line, v, ' ');
      e.u = std::stoi(v[0]);
      e.v = std::stoi(v[1]);
      e.time = std::stoi(v[2]);
      edgeList.push_back(e);
    }
    dataFileG.close();
  } else {
    std::cerr << "Error: could not open provided graph data file." << std::endl;
    return 1;
  }
  std::ifstream dataFileM(argv[2]);
  if (dataFileM.is_open()) {
    size_t i = 0;
    while (std::getline(dataFileM, line)) {
      std::vector<std::string> v;
      Edge e;
      split(line, v, ' ');
      if (i > motif.size()) {
        std::cerr <<
            "Error: motif is too large. Recompile with larger MOTIF_SIZE." <<
            std::endl;
        return 1;
      }
      e.u = std::stoi(v[0]);
      e.v = std::stoi(v[1]);
      e.time = std::stoi(v[2]);
      motif.push_back(e);
      i++;
    }
    dataFileM.close();
  } else {
    std::cerr << "Error: could not open provided motif data file." << std::endl;
    return 1;
  }
  return 0;
}

int main(int argc, char** argv) {
  std::cout << "Constructing Mint." << std::endl;
  Mint mint;
  std::cout << "Loading files." << std::endl;
  int result = loadFiles(argc, argv, mint.edgeList, mint.tM.motif);
  if (result != 0) {
    return result;
  }
  std::cout << "Setting up Mint." << std::endl;
  mint.setup();
  std::cout << "Running Mint." << std::endl;
  mint.run();
  return 0;
}
