// Definitions for Mint simulator

#include <iostream>
#include <algorithm>
#include "mint.hpp"

bool Task::isMapped(int gN, int mN) {
  bool result = false;
  for (size_t i = 0; i < nodeMap.size(); i++) {
    if (nodeMap[i].gNode == gN && nodeMap[i].mNode == mN) {
      if (nodeMap[i].count < 1) {
        std::cerr << "Error: found a zero-count mapping" << std::endl;
      }
      result = true;
      break;
    }
  }
  return result;
}

bool Task::hasMap(int gN) {
  bool result = false;
  for (size_t i = 0; i < nodeMap.size(); i++) {
    if (nodeMap[i].gNode == gN) {
      result = true;
      break;
    }
  }
  return result;
}

void Task::insertMapping(int gN, int mN) {
  if (VERBOSE) std::cout << "Inserting mapping between graph " << gN <<
                   " and motif " << mN << std::endl;
  bool exists = false;
  for (size_t i = 0; i < nodeMap.size(); i++) {
    if (nodeMap[i].gNode == gN && nodeMap[i].mNode == mN) {
      if (nodeMap[i].count < 1) {
        std::cerr << "Error: found a zero-count mapping" << std::endl;
      }
      nodeMap[i].count++;
      exists = true;
      if (VERBOSE) std::cout << "Incrementing existing mapping" << std::endl;
      break;    
    }
  }
  if (!exists) {
    for (size_t i = 0; i < nodeMap.size(); i++) {
      if (nodeMap[i].gNode == gN || nodeMap[i].mNode == mN) {
        nodeMap.erase(nodeMap.begin() + i);
        i--;
        if (VERBOSE) std::cout << "Erased a mapping" << std::endl;
      }
    }
    nodeMap.push_back(Mapping(mNode, gNode, 1));
    if (VERBOSE) std::cout << "Pushing new mapping" << std::endl;
  }
  return;
}

void Task::removeMapping(int gN, int mN) {
  if (VERBOSE) std::cout << "Removing mapping between graph " << gN <<
                   " and motif " << mN << std::endl;
  bool removed = false;
  for (size_t i = 0; i < nodeMap.size(); i++) {
    if (nodeMap[i].gNode == gN && nodeMap[i].mNode == mN) {
      if (nodeMap[i].count == 1) {
        nodeMap.erase(nodeMap.begin() + i);
        i--;
        removed = true;
        if (VERBOSE) std::cout << "Erased a mapping" << std::endl;
      } else if (nodeMap[i].count == 0) {
        std::cerr << "Error: found a zero-count mapping" << std::endl;
      } else {
        nodeMap[i].count--;
        removed = true;
        if (VERBOSE) std::cout << "Decremented a mapping" << std::endl;
      }
    }
  }
  if (!removed) {
    std::cerr << "Error: tried to remove a non-existent mapping." << std::endl;
  }
  return;
}

void TaskQueue::setup(std::vector<Edge>& edgeList, std::vector<Edge>& motif) {
  for (size_t i = 0; i < edgeList.size(); i++) {
    Task t;
    t.eG = i;
    t.eM = 0;
    t.uG = edgeList[i].u;
    t.vG = edgeList[i].v;
    t.uM = motif[0].u;
    t.vM = motif[0].v;
    t.type = bookkeep;
    tasks.push(t);
  }
  if (VERBOSE) std::cout << "Pushed " << tasks.size() << " root tasks" <<
                   std::endl;
  return;
}

void MappingStore::addResult(ContextMem& cMem) {
  store.push_back(cMem.nodeMap);
  // need to make sure this saves a copy not a reference
  return;
}

MgrStatus ContextMgr::updateContext(Task& task) {
  MgrStatus status;
  cMem.busy = true;
  cMem.eG = task.eG;
  cMem.eM = task.eM;
  switch (task.type) {
    case bookkeep:
      if (VERBOSE) std::cout << "Context manager bookkeeping" << std::endl;
      if (task.eM == motifSize - 1) {
        if (VERBOSE) std::cout << "Motif found, saving and backtracking" <<
                         std::endl;
        status = remanage; // Motif found, step back to continue search
        cMem.nodeMap = task.nodeMap;
        results.addResult(cMem);
      } else {
        if (VERBOSE) std::cout << "Bookkeeping mapped edge " << task.eG <<
                         std::endl;
        status = dispatch;
        cMem.uG = edgeList[task.eG].u;
        cMem.vG = edgeList[task.eG].v;
        cMem.uM = task.uM;
        cMem.vM = task.vM;
        task.insertMapping(cMem.uG, cMem.uM);
        task.insertMapping(cMem.vG, cMem.vM);
        cMem.nodeMap = task.nodeMap;
        if (cMem.eStack.empty()) {
          cMem.time = edgeList[task.eG].time + motifTime;
          if (VERBOSE) std::cout << "Set time bound: " << cMem.time <<
                           std::endl;
        }
        cMem.eStack.push(task.eG);
        cMem.eM += 1;
        cMem.eG += 1;
        cMem.busy = false;
        // need to figure out correct eM and eG management between task and cMem
      }
      break;
    case backtrack:
      if (VERBOSE) std::cout << "Context manager backtracking, current eG " <<
                       cMem.eG << std::endl;
      cMem.eG += 1;
      if (VERBOSE) std::cout << "New eG is " << cMem.eG << std::endl;
      while (cMem.eG >= edgeList.size() || edgeList[cMem.eG].time > cMem.time) {
        if (!(cMem.eStack.size() == 1)) {
          status = dispatch;
          cMem.eG = cMem.eStack.top() + 1;
          if (VERBOSE) std::cout << "Reset eG to " << cMem.eG << std::endl;
          cMem.eStack.pop();
          if (cMem.eStack.empty()) {
            cMem.time = INT_MAX;
          }
          task.removeMapping(cMem.uG, cMem.uM);
          task.removeMapping(cMem.vG, cMem.vM);
          cMem.nodeMap = task.nodeMap;
          cMem.eM--;
          if (VERBOSE) std::cout << "Backtrack done, new eM " << cMem.eM <<
                           std::endl;
        } else {
          if (VERBOSE) std::cout <<
                           "Backtrack on root edge, search tree complete" <<
                           std::endl;
          status = end;
          cMem.eStack.pop();
          break;
        }
      }
      break;
    default:
      status = end;
      std::cerr << "Error: updateContext received invalid task type" <<
          std::endl;
  }
  return status;
}

void Dispatcher::dispatch(Task& task) {
  task.type = search;
  // Motif edge to map, incremented only by bookkeep
  task.eM = cMem.eM;
  // Graph edge to search for next edge from, set by backtrack
  task.eG = cMem.eG;
  task.uM = tM.motif[task.eM].u;
  task.vM = tM.motif[task.eM].v;
  auto iterator = std::ranges::find_if(cMem.nodeMap.begin(), cMem.nodeMap.end(),
                                       [&](Mapping i) {
                                         return i.mNode == task.uM;
                                       });
  if (iterator != cMem.nodeMap.end()) {
    task.uG = iterator->gNode;
    if (VERBOSE) std::cout << "Found that uM " << task.uM << " mapped to uG " <<
                     task.uG << std::endl;
  } else {
    task.uG = -1;
    if (VERBOSE) std::cout << "No mapping found for uM " << task.uM <<
                     std::endl;
  }
  iterator = std::ranges::find_if(cMem.nodeMap.begin(), cMem.nodeMap.end(),
                                  [&](Mapping i) {
                                    return i.mNode == task.vM;
                                  });
  if (iterator != cMem.nodeMap.end()) {
    task.vG = iterator->gNode;
    if (VERBOSE) std::cout << "Found that vM " << task.vM << " mapped to vG " <<
                     task.vG << std::endl;
  } else {
    task.vG = -1;
    if (VERBOSE) std::cout << "No mapping found for vM " << task.vM <<
                     std::endl;
  }
  task.nodeMap = cMem.nodeMap;
  task.time = cMem.time;
  return;
}

std::vector<size_t> SearchEng::searchPhaseOne(Task& task) {
  if (VERBOSE) std::cout << "Beginning search phase one" << std::endl;
  if (VERBOSE) std::cout << "eM " << task.eM << " and eG " << task.eG <<
                   std::endl;
  std::vector<size_t> fEdges;
  bool uCheck = (task.uG >= 0);
  bool vCheck = (task.vG >= 0);
  for (size_t i = 0; i < edgeList.size(); i++) {
    if ((!uCheck || edgeList[i].u == task.uG)
        && (!vCheck || edgeList[i].v == task.vG)) {
      fEdges.push_back(i);
    }
  }
  if (VERBOSE) std::cout << "Adjacency filtering gives " << fEdges.size() <<
                   " edges" << std::endl;
  for (size_t i = 0; i < fEdges.size(); i++) {
    if (fEdges[i] < task.eG) {
      fEdges.erase(fEdges.begin() + i);
      i--;
    }
  }
  if (VERBOSE) std::cout << "Time order filtering gives " << fEdges.size() <<
                   " edges" << std::endl;
  if (VERBOSE) std::cout << "Phase one results:" << std::endl;
  for (size_t i = 0; i < fEdges.size(); i++) {
    if (VERBOSE) std::cout << edgeList[fEdges[i]].u << " " <<
                     edgeList[fEdges[i]].v << " " <<
                     edgeList[fEdges[i]].time << std::endl;
  }
  return fEdges;
}

void SearchEng::searchPhaseTwo(Task& task, std::vector<size_t> fEdges) {
  if (VERBOSE) std::cout << "Beginning search phase two" << std::endl;
  // Fetch full edge data
  std::vector<Edge> fEdgesData;
  for (size_t i = 0; i < fEdges.size(); i++) {
    fEdgesData.push_back(edgeList[fEdges[i]]);
  }
  for (size_t i = 0; i < fEdgesData.size(); i++) {
    if (fEdgesData[i].time <= task.time) {
      if ((task.isMapped(fEdgesData[i].u, task.uM)
           || !task.hasMap(fEdgesData[i].u))
          && (task.isMapped(fEdgesData[i].v, task.vM)
              || !task.hasMap(fEdgesData[i].v))) {
        task.eG = fEdges[i];
        task.type = bookkeep;
        if (VERBOSE) std::cout << "Edge match found" << std::endl;
        return;
      }
    }
  }
  if (VERBOSE) std::cout << "Edge match not found" << std::endl;
  task.eG = edgeList.size();
  task.type = backtrack;
  return;
}

void ComputeUnit::executeRootTask(Task t) {
  bool working = true;
  while (working) {
    MgrStatus mStatus;
    if (VERBOSE) std::cout << "Updating context" << std::endl;
    mStatus = cMgr.updateContext(t);
    switch (mStatus) {
      case end:
        if (VERBOSE) std::cout << "Manager status: end" << std::endl;
        working = false;
        break;
      case dispatch:
        if (VERBOSE) std::cout << "Manager status: dispatch" << std::endl;
        disp.dispatch(t);
        if (VERBOSE) std::cout << "Beginning search" << std::endl;
        sEng.searchPhaseTwo(t, sEng.searchPhaseOne(t));
        break;
      case remanage:
        if (VERBOSE) std::cout << "Manager status: remanage" << std::endl;
        t.type = backtrack;
        break;
      default:
        std::cerr << "Error: unrecognized Context Manager status code" <<
            std::endl;
    }
  }
  return;
}

Mint::Mint(TargetMotif m, std::vector<Edge> e) {
  tM = m;
  edgeList = e;
  tM.time = tM.motif.back().time - tM.motif.front().time;
  if (VERBOSE) std::cout << "Target motif is " << tM.motif.size() <<
                   " edges and " << tM.time << " timesteps long" << std::endl;
  for (size_t i = 0; i < NUM_CUS; i++) {
    cMems.push_back(ContextMem());
    cUnits.push_back(ComputeUnit(results, tM, edgeList, cMems.back()));
    cUnits.back().cMgr.motifSize = tM.motif.size();
    cUnits.back().cMgr.motifTime = tM.time;
  }
  tQ.setup(edgeList, tM.motif);  
}

void Mint::printResults() {
  std::cout << "Results:" << std::endl;
  for (size_t i = 0; i < results.store.size(); i++) {
    for (size_t j = 0; j < results.store[i].size(); j++) {
      std::cout << results.store[i][j].mNode << " " <<
                       results.store[i][j].gNode << " " <<
                       results.store[i][j].count << std::endl;
    }
    std::cout << "--------------------" << std::endl;
  }
  return;
}

void Mint::run() {
  while (!tQ.tasks.empty()) {
    // Find CU that is earliest in time to give a task to
    int nextCU = 0;
    int minCycles = INT_MIN;
    for (size_t i = 0; i < NUM_CUS; i++) {
      if (cUnits[i].cycles < minCycles) {
        nextCU = i;
        minCycles = cUnits[i].cycles;
      }
    }
    if (minCycles == INT_MIN) {
      minCycles = 0;
    }
    if (VERBOSE) std::cout << "Executing root task " << tQ.tasks.front().eG <<
                     " with CU " << nextCU << " at cycle " << minCycles <<
                     std::endl;
    cUnits[nextCU].executeRootTask(tQ.tasks.front());
    tQ.tasks.pop();
    if (VERBOSE) std::cout << "----------------------------------------"
                           << std::endl;
  }
  // Collect cycle stats
  int maxCycles = INT_MIN;
  int totalCycles = 0;
  for (size_t i = 0; i < NUM_CUS; i++) {
    if (cUnits[i].cycles > maxCycles) {
      maxCycles = cUnits[i].cycles;
    }
    totalCycles += cUnits[i].cycles;
  }
  std::cout << "Total cycles taken: " << totalCycles << std::endl;
  std::cout << "End-to-end cycle count: " << maxCycles << std::endl;
  printResults();
  return;
}
