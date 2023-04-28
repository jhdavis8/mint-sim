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
      if (task.eM == motifSize - 1) {
        status = end;
        mStr.addResult(task);
      } else {
        status = dispatch;
        uG = edgeList[eG].u;
        vG = edgeList[eG].v;
        uM = edgeList[eM].u;
        vM = edgeList[eM].v;
        cMem.nodeMap.push_back(Mapping(uM, uG, 0));
        cMem.nodeMap.push_back(Mapping(vM, vG, 0));
        for (int i = 0; i < cMem.nodeMap.length(); i++) {
          if (cMem.nodeMap[i].gNode == uG) {
            cMem.nodeMap[i].count++;
          }
          if (cMem.nodeMap[i].gNode == vG) {
            cMem.nodeMap[i].count++;
          }
        }
        if (cMem.eStack.isEmpty()) {
          cMem.time = edgeList[eG].time + motifTime;
        }
        cMem.eStack.push(eG);
        cMem.eM += 1; // is this correct/necessary in the parallel context?
        // need to figure out correct eM and eG management between task and cMem
        // Also what about busy?
      }
      break;
    case backtrack:
      cMem.eG += 1;
      while (eG > edgeList.length() || edgeList[eG].time > cMem.time) {
        if (!eStack.isEmpty()) {
          status = dispatch;
          cMem.eG = eStack.pop() + 1;
          if (eStack.isEmpty()) {
            cMem.time = INT_MAX;
          }
          
        }
      }
      break;
    default:
      std::cout << "Error: updateContext received invalid task type." << std::endl;
  return status;
}
