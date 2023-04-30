// Definitions for Mint simulator

#include <stdio>
#include "mint.hpp"

void MappingStore::addResult(ContextMem& cMem) {
  store.push_back(cMem.nodeMap); // need to make sure this saves a copy not a reference
  return;
}

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
        status = backtrack;
        mStr.addResult(task);
      } else {
        status = dispatch;
        uG = task.uG;
        vG = task.vG;
        uM = task.uM;
        vM = task.vM;
        bool uG_found = false;
        bool vG_found = false;
        for (int i = 0; i < cMem.nodeMap.length(); i++) {
          if (cMem.nodeMap[i].gNode == uG) {
            cMem.nodeMap[i].count++;
            uG_found = true;
          }
          if (cMem.nodeMap[i].gNode == vG) {
            cMem.nodeMap[i].count++;
            vG_found = true;
          }
        }
        if (!uG_found) {
          cMem.nodeMap.push_back(Mapping(uM, uG, 1));
        }
        if (!vG_found) {
          cMem.nodeMap.push_back(Mapping(vM, vG, 0));
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
          cMem.eG = eStack.pop() + 1; // why add one here?
          if (eStack.isEmpty()) {
            cMem.time = INT_MAX;
          }
          for (int i = 0; i < cMem.nodeMap.length(); i++) {
            if (cMem.nodeMap[i].gNode == uG) {
              cMem.nodeMap[i].count--;
            }
            if (cMem.nodeMap[i].gNode == vG) {
              cMem.nodeMap[i].count--;
            }
            if (cMem.nodeMap[i].count == 0) {
              cMem.nodeMap.erase(i);
              i--;
            }
          }
          cMem.eM--;
        } else {
          status = end;
          break;
        }
      }
      break;
    default:
      std::cout << "Error: updateContext received invalid task type." << std::endl;
  }
  return status;
}

void Dispatcher::setup(ContextMem& c, TargetMotif& m, int& cyc) {
  cMem = c;
  TargetMotif = m;
  cycles = cyc;
  return;
}

void Dispatcher::dispatch(Task& task) {
  task.type = search;
  task.eM = cMem.eM;
  task.eG = cMem.eG;
  task.uM = tM.motif[eM].u;
  task.vM = tM.motif[eM].v;
  auto iterator = std::ranges:find_if(cMem.nodeMap.begin(), cMem.nodeMap.end(),
                                      [](Mapping i) { return i.mNode == task.uM; });
  if (iterator != cMem.nodeMap.end()) {
    task.uG = *iterator;
  } else {
    task.uG = -1;
  }
  auto iterator = std::ranges:find_if(cMem.nodeMap.begin(), cMem.nodeMap.end(),
                                      [](Mapping i) { return i.mNode == task.vM; });
  if (iterator != cMem.nodeMap.end()) {
    task.vG = *iterator;
  } else {
    task.vG = -1;
  }
  task.time = cMem.time;
  return;
}

void SearchEng::setup(ContextMem& c, std::vector<Edge>& eL, int& cyc) {
  cMem = c;
  cycles = cyc;
  edgeList = eL;
  return;
}

std::vector<Edge> searchPhaseOne(Task& task) {
  std::vector<Edge> fEdges;
  for (int i = 0; i < edgeList.length(); i++) {
    if (task.uG >= 0 && task.vG >= 0) {
      if (edgeList[i].u == task.uG && edgeList[i].v == task.vG) {
        fEdges.push_back(edgeList[i]);
      }
    } else if (task.uG >= 0) {
      if (edgeList[i].u == task.uG) {
        fEdges.push_back(edgeList[i]);
      }
    } else if (task.vG >= 0) {
      if (edgeList[i].v == task.vG) {
        fEdges.push_back(edgeList[i]);
      }
    }
  }
  for (int i = 0; i < fEdges.length(); i++) {
    if (fEdges[i].time < edgeList[eG].time) {
      fEdges.erase(i);
      i--;
    }
  }
}
