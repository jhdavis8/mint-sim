// Definitions for Mint simulator

#include <stdio>
#include "mint.hpp"

void ContextMgr::setup(ContextMem& c, MappingStore& m, std::vector<Edge>& eL, int& cyc) {
  cMem = c;
  mStr = m;
  edgeList = eL;
  cycles = cyc;
  return;
}

MgrStatus ContextMgr::updateContext(Task& task) {
  MgrStatus status;
  cMem.busy = true;
  switch (task.type) {
    case bookkeep:
      // do bookkeeping
      if (task.eM == motifSize - 1) {
        status = end;
        mStr.addResult(task);
      } else {
        uG = edgeList[eG].u;
        vG = edgeList[eG].v;
        uM = edgeList[eM].u;
        vM = edgeList[eM].v;
        // Need to check if node are already mapped
        cMem.nodeMap.push_back(Mapping();
      }
      break;
    case backtrack:
      // do backtracking
      break;
    default:
      std::cout << "Error: updateContext received invalid task type." << std::endl;
  return status;
}
