// Definitions for Mint simulator

#include <iostream>
#include <algorithm>
#include "mint.hpp"

bool Task::isMapped(int gN, int mN) {
  bool result = false;
  for (size_t i = 0; i < nodeMap.size(); i++) {
    if (nodeMap[i].gNode == gN && nodeMap[i].mNode == mN) {
      if (nodeMap[i].count < 1) {
        std::cerr << "Error: found a zero-count mapping in search" <<
            std::endl;
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
      if (nodeMap[i].count < 1) {
        std::cerr << "Error: found a zero-count mapping in search" <<
            std::endl;
      }
      result = true;
      break;
    }
  }
  return result;
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
  std::cout << "Pushed " << tasks.size() << " root tasks" << std::endl;
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
  switch (task.type) {
    case bookkeep:
      std::cout << "Context manager bookkeeping" << std::endl;
      if (task.eM == motifSize - 1) {
        std::cout << "Motif found, saving and backtracking" << std::endl;
        status = remanage; // Motif found, step back to continue search
        cMem.nodeMap = task.nodeMap;
        results.addResult(cMem);
      } else {
        std::cout << "Bookkeeping mapped edge " << task.eG << std::endl;
        status = dispatch;
        int uG = edgeList[task.eG].u;
        int vG = edgeList[task.eG].v;
        int uM = task.uM; // Motif data is correct from last update by dispatch
        int vM = task.vM;
        cMem.nodeMap = task.nodeMap;
        if (!task.isMapped(uM, uG)) {
          std::cout << "Pushing new u mapping: motif node " << uM <<
              " to graph node " << uG << std::endl;
          cMem.nodeMap.push_back(Mapping(uM, uG, 0));
        }
        if (!task.isMapped(vM, vG)) {
          std::cout << "Pushing new v mapping: motif node " << vM <<
              " to graph node " << vG << std::endl;
          cMem.nodeMap.push_back(Mapping(vM, vG, 0));
        }
        std::cout <<
            "Incrementing mapped edge counts for any mapped graph nodes" <<
            std::endl;
        std::cout << "nodeMap has " << cMem.nodeMap.size() << " entries" <<
            std::endl;
        for (size_t i = 0; i < cMem.nodeMap.size(); i++) {
          if (cMem.nodeMap[i].gNode == uG) {
            cMem.nodeMap[i].count++;
          }
          if (cMem.nodeMap[i].gNode == vG) {
            cMem.nodeMap[i].count++;
          }
        }
        if (cMem.eStack.empty()) {
          cMem.time = edgeList[task.eG].time + motifTime;
          std::cout << "Set time bound: " << cMem.time << std::endl;
        }
        cMem.eStack.push(task.eG);
        cMem.eM += 1;
        cMem.eG += 1;
        cMem.busy = false;
        // need to figure out correct eM and eG management between task and cMem
      }
      break;
    case backtrack:
      std::cout << "Context manager backtracking, current eG " <<
          cMem.eG << std::endl;
      cMem.eG += 1;
      std::cout << "New eG is " << cMem.eG << std::endl;
      while (cMem.eG >= edgeList.size() || edgeList[cMem.eG].time > cMem.time) {
        if (!(cMem.eStack.size() == 1)) {
          status = dispatch;
          cMem.eG = cMem.eStack.top() + 1;
          cMem.eStack.pop();
          if (cMem.eStack.empty()) {
            cMem.time = INT_MAX;
          }
          for (size_t i = 0; i < cMem.nodeMap.size(); i++) {
            if (cMem.nodeMap[i].gNode == task.uG) {
              cMem.nodeMap[i].count--;
            }
            if (cMem.nodeMap[i].gNode == task.vG) {
              cMem.nodeMap[i].count--;
            }
            if (cMem.nodeMap[i].count == 0) {
              cMem.nodeMap.erase(cMem.nodeMap.begin() + i);
              i--;
            }
          }
          cMem.eM--;
          std::cout << "Backtrack done, new eM " << cMem.eM << std::endl;
        } else {
          std::cout << "Backtrack on root edge, search tree complete" <<
              std::endl;
          status = end;
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
  } else {
    task.uG = -1;
  }
  iterator = std::ranges::find_if(cMem.nodeMap.begin(), cMem.nodeMap.end(),
                                  [&](Mapping i) {
                                    return i.mNode == task.vM;
                                  });
  if (iterator != cMem.nodeMap.end()) {
    task.vG = iterator->gNode;
  } else {
    task.vG = -1;
  }
  task.nodeMap = cMem.nodeMap;
  task.time = cMem.time;
  return;
}

std::vector<size_t> SearchEng::searchPhaseOne(Task& task) {
  std::cout << "Beginning search phase one" << std::endl;
  std::cout << "eM " << task.eM << " and eG " << task.eG << std::endl;
  std::vector<size_t> fEdges;
  for (size_t i = 0; i < edgeList.size(); i++) {
    if ((task.uG >= 0 && task.vG >= 0)
        && (edgeList[i].u == task.uG && edgeList[i].v == task.vG)) {
      fEdges.push_back(i);
    } else if (task.uG >= 0 && edgeList[i].u == task.uG) {
      fEdges.push_back(i);
    } else if (task.vG >= 0 && edgeList[i].v == task.vG) {
      fEdges.push_back(i);
    } else if (task.uG < 0 && task.vG < 0) {
      fEdges.push_back(i);
    }
  }
  std::cout << "Adjacency filtering gives " << fEdges.size() << " edges" <<
      std::endl;
  for (size_t i = 0; i < fEdges.size(); i++) {
    if (fEdges[i] < task.eG) {
      fEdges.erase(fEdges.begin() + i);
      i--;
    }
  }
  std::cout << "Time order filtering gives " << fEdges.size() << " edges" <<
      std::endl;
  std::cout << "Phase one results:" << std::endl;
  for (size_t i = 0; i < fEdges.size(); i++) {
    std::cout << edgeList[fEdges[i]].u << " " << edgeList[fEdges[i]].v << " " <<
        edgeList[fEdges[i]].time << std::endl;
  }
  return fEdges;
}

void SearchEng::searchPhaseTwo(Task& task, std::vector<size_t> fEdges) {
  std::cout << "Beginning search phase two" << std::endl;
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
        std::cout << "Edge match found" << std::endl;
        return;
      }
    }
  }
  std::cout << "Edge match not found" << std::endl;
  task.eG = edgeList.size();
  task.type = backtrack;
  return;
}

void ComputeUnit::executeRootTask(Task t) {
  bool working = true;
  while (working) {
    MgrStatus mStatus;
    std::cout << "Updating context" << std::endl;
    mStatus = cMgr.updateContext(t);
    switch (mStatus) {
      case end:
        std::cout << "Manager status: end" << std::endl;
        working = false;
        break;
      case dispatch:
        std::cout << "Manager status: dispatch" << std::endl;
        disp.dispatch(t);
        std::cout << "Beginning search" << std::endl;
        sEng.searchPhaseTwo(t, sEng.searchPhaseOne(t));
        break;
      case remanage:
        std::cout << "Manager status: remanage" << std::endl;
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
  std::cout << "Target motif is " << tM.motif.size() << " edges and " <<
      tM.time << " timesteps long" << std::endl;
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
    std::cout << "Executing root task " << tQ.tasks.front().eG << " with CU " <<
        nextCU << " at cycle " << minCycles << std::endl;
    cUnits[nextCU].executeRootTask(tQ.tasks.front());
    tQ.tasks.pop();
    std::cout <<
        "--------------------------------------------------------------------------------"
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
