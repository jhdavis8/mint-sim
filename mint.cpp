// Definitions for Mint simulator

#include <algorithm>
#include "mint.hpp"

bool Task::isMapped(int gN, int mN) {
  bool result = false;
  for (size_t i = 0; i < nodeMap.size(); i++) {
    if (nodeMap.at(i).gNode == gN && nodeMap.at(i).mNode == mN) {
      if (nodeMap.at(i).count < 1) {
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
    if (nodeMap.at(i).gNode == gN) {
      result = true;
      break;
    }
  }
  return result;
}

void Task::insertMapping(int gN, int mN, size_t& cycles) {
  if (VERBOSE) std::cout << "Inserting mapping between graph " << gN <<
                   " and motif " << mN << std::endl;
  bool exists = false;
  for (size_t i = 0; i < nodeMap.size(); i++) {
    if (nodeMap.at(i).gNode == gN && nodeMap.at(i).mNode == mN) {
      if (nodeMap.at(i).count < 1) {
        std::cerr << "Error: found a zero-count mapping" << std::endl;
      }
      nodeMap.at(i).count++;
      cycles += CMEM_LATENCY + ADD_LATENCY;
      exists = true;
      if (VVERBOSE) std::cout << "Incrementing existing mapping" << std::endl;
      break;
    }
  }
  if (!exists) {
    for (size_t i = 0; i < nodeMap.size(); i++) {
      if (nodeMap.at(i).gNode == gN || nodeMap.at(i).mNode == mN) {
        nodeMap.erase(nodeMap.begin() + i);
        i--;
        if (VVERBOSE) std::cout << "Erased a mapping" << std::endl;
      }
    }
    nodeMap.push_back(Mapping(mN, gN, 1));
    cycles += CMEM_LATENCY*3;
    if (VVERBOSE) std::cout << "Pushing new mapping" << std::endl;
  }
  return;
}

void Task::removeMapping(int gN, int mN, size_t& cycles) {
  if (VERBOSE) std::cout << "Removing mapping between graph " << gN <<
                   " and motif " << mN << std::endl;
  bool removed = false;
  for (size_t i = 0; i < nodeMap.size(); i++) {
    if (nodeMap.at(i).gNode == gN && nodeMap.at(i).mNode == mN) {
      if (nodeMap.at(i).count == 1) {
        nodeMap.erase(nodeMap.begin() + i);
        i--;
        cycles += CMEM_LATENCY*3;
        removed = true;
        if (VVERBOSE) std::cout << "Erased a mapping" << std::endl;
      } else if (nodeMap.at(i).count == 0) {
        std::cerr << "Error: found a zero-count mapping" << std::endl;
      } else {
        nodeMap.at(i).count--;
        cycles += CMEM_LATENCY + ADD_LATENCY;
        removed = true;
        if (VVERBOSE) std::cout << "Decremented a mapping" << std::endl;
      }
    }
  }
  if (!removed) {
    std::cerr << "Error: tried to remove a non-existent mapping." << std::endl;
    std::cerr << "gN = " << gN << ", mN = " << mN << std::endl;
    printNodeMap();
    throw "Invalid remove call";
  }
  return;
}

void Task::printNodeMap() {
  std::cout << "nodeMap:" << std::endl;
  for (size_t i = 0; i < nodeMap.size(); i++) {
    std::cout << "G " << nodeMap.at(i).gNode << " M " << nodeMap.at(i).mNode
              << " C " << nodeMap.at(i).count << std::endl;
  }
}

void TaskQueue::setup(std::vector<Edge>& edgeList, std::vector<Edge>& motif) {
  for (size_t i = 0; i < edgeList.size(); i++) {
    Task t;
    t.eG = i;
    t.eM = 0;
    t.uG = edgeList.at(i).u;
    t.vG = edgeList.at(i).v;
    t.uM = motif.at(0).u;
    t.vM = motif.at(0).v;
    t.type = bookkeep;
    tasks.push(t);
  }
  if (VERBOSE) std::cout << "Pushed " << tasks.size() << " root tasks" <<
                   std::endl;
  return;
}

void MappingStore::addResult(ContextMem& cMem) {
#pragma omp critical
  store.push_back(cMem.nodeMap);
  return;
}

MgrStatus ContextMgr::updateContext(Task& task) {
  MgrStatus status;
  cMem.busy = true;
  cMem.eG = task.eG;
  cMem.eM = task.eM;
  cycles += CMEM_LATENCY*3 + JMP_LATENCY;
  switch (task.type) {
    case bookkeep:
     if (VERBOSE) std::cout << "Context manager bookkeeping" << std::endl;
      if (task.eM == motifSize - 1) {
        if (VERBOSE) std::cout << "Motif found, saving and backtracking" <<
                         std::endl;
        status = remanage; // Motif found, step back to continue search
        cMem.nodeMap = task.nodeMap;
        results.addResult(cMem);
        cycles += CMEM_LATENCY*3*task.nodeMap.size();
        cycles += DRAM_LATENCY*3*task.nodeMap.size();
      } else {
        if (VERBOSE) std::cout << "Bookkeeping mapped edge " << task.eG <<
                         std::endl;
        status = dispatch;
        cMem.uG = edgeList.at(task.eG).u;
        cMem.vG = edgeList.at(task.eG).v;
        cMem.uM = task.uM;
        cMem.vM = task.vM;
        cycles += CMEM_LATENCY*4 + CACHE_EXP*2;
        task.insertMapping(cMem.uG, cMem.uM, cycles);
        task.insertMapping(cMem.vG, cMem.vM, cycles);
        cMem.nodeMap = task.nodeMap;
        cycles += JMP_LATENCY;
        if (cMem.eStack.empty()) {
          cMem.time = edgeList.at(task.eG).time + motifTime;
          cycles += CMEM_LATENCY + ADD_LATENCY + CACHE_EXP;
          if (VVERBOSE) std::cout << "Set time bound: " << cMem.time <<
                           std::endl;
        }
        cMem.eStack.push(task.eG);
        cMem.eM += 1;
        cMem.eG += 1;
        cMem.busy = false;
        cycles += CMEM_LATENCY*4 + ADD_LATENCY*2;
        // need to figure out correct eM and eG management between task and cMem
      }
      break;
    case backtrack:
      if (VERBOSE) std::cout << "Context manager backtracking, current eG " <<
                       cMem.eG << std::endl;
      cMem.eG += 1;
      cycles += CMEM_LATENCY + ADD_LATENCY;
      if (VVERBOSE) std::cout << "New eG is " << cMem.eG << std::endl;
      while (cMem.eG >= edgeList.size() || edgeList.at(cMem.eG).time > cMem.time) {
        cycles += JMP_LATENCY*2 + CMEM_LATENCY*2 + CACHE_EXP;
        if (!(cMem.eStack.size() == 1)) {
          status = dispatch;
          cMem.eG = cMem.eStack.top() + 1;
          cycles += CMEM_LATENCY*2 + ADD_LATENCY;
          if (VVERBOSE) std::cout << "Reset eG to " << cMem.eG << std::endl;
          cMem.eStack.pop();
          if (cMem.eStack.empty()) {
            cMem.time = INT_MAX;
            std::cerr << "Popped the last edge, this shouldn't happen" <<
                std::endl;
            throw "Invalid edge pop";
          }
          //int last_eG = cMem.eG - 1;
          cMem.eM--;
          cycles += ADD_LATENCY*2 + CMEM_LATENCY*2;
          /*
          cMem.uG = edgeList[last_eG].u;
          cMem.vG = edgeList[last_eG].v;
          cMem.uM = ;
          cMem.vM = ;
          */
          task.removeMapping(cMem.uG, cMem.uM, cycles);
          task.removeMapping(cMem.vG, cMem.vM, cycles);
          cMem.nodeMap = task.nodeMap;
          if (VVERBOSE) std::cout << "Backtrack done, new eM " << cMem.eM <<
                           std::endl;
        } else {
          if (VVERBOSE) std::cout <<
                           "Backtrack on root edge, search tree complete" <<
                           std::endl;
          status = end;
          cMem.eStack.pop();
          cycles += CMEM_LATENCY;
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
  task.uM = tM.motif.at(task.eM).u;
  task.vM = tM.motif.at(task.eM).v;
  cycles += CMEM_LATENCY*4;
  auto iterator = std::ranges::find_if(cMem.nodeMap.begin(), cMem.nodeMap.end(),
                                       [&](Mapping i) {
                                         return i.mNode == task.uM;
                                       });
  if (iterator != cMem.nodeMap.end()) {
    task.uG = iterator->gNode;
    if (VVERBOSE) std::cout << "Found that uM " << task.uM << " mapped to uG " <<
                     task.uG << std::endl;
  } else {
    task.uG = -1;
    if (VVERBOSE) std::cout << "No mapping found for uM " << task.uM <<
                     std::endl;
  }
  iterator = std::ranges::find_if(cMem.nodeMap.begin(), cMem.nodeMap.end(),
                                  [&](Mapping i) {
                                    return i.mNode == task.vM;
                                  });
  if (iterator != cMem.nodeMap.end()) {
    task.vG = iterator->gNode;
    if (VVERBOSE) std::cout << "Found that vM " << task.vM << " mapped to vG " <<
                     task.vG << std::endl;
  } else {
    task.vG = -1;
    if (VVERBOSE) std::cout << "No mapping found for vM " << task.vM <<
                     std::endl;
  }
  cycles += JMP_LATENCY*2 + CMEM_LATENCY*2 + MOV_LATENCY*2;
  task.nodeMap = cMem.nodeMap;
  task.time = cMem.time;
  cycles += CMEM_LATENCY*2;
  return;
}

std::vector<size_t> SearchEng::searchPhaseOne(Task& task) {
  if (VVERBOSE) std::cout << "Beginning search phase one" << std::endl;
  if (VVERBOSE) std::cout << "eM " << task.eM << " and eG " << task.eG <<
                   std::endl;
  std::vector<size_t> fEdges;
  bool uCheck = (task.uG >= 0);
  bool vCheck = (task.vG >= 0);
  cycles += MOV_LATENCY*2;
  for (size_t i = 0; i < edgeList.size(); i++) {
    if ((!uCheck || edgeList.at(i).u == task.uG)
        && (!vCheck || edgeList.at(i).v == task.vG)) {
      fEdges.push_back(i);
    }
  }
  // Linear search in parallel, accrue latency once per cache line
  cycles += (CACHE_EXP*2 + JMP_LATENCY*2 + MOV_LATENCY + ADD_LATENCY)*(edgeList.size()/8);
  std::vector<size_t> fEdges2;
  if (VVERBOSE) std::cout << "Adjacency filtering gives " << fEdges.size() <<
                   " edges" << std::endl;
  bool recorded = false;
  for (size_t i = memo.getStart(uCheck, vCheck, task.uG, task.vG, task.eG, fEdges.size(), cycles);
       i < fEdges.size(); i++) {
    if (fEdges.at(i) >= task.eG) {
      fEdges2.push_back(fEdges.at(i));
    }
    memo.record(uCheck, vCheck, task.uG, task.vG, root_eG, fEdges.at(i), i,
                recorded, fEdges.size(), cycles);
    cycles += JMP_LATENCY*2 + MOV_LATENCY + CACHE_EXP + ADD_LATENCY;
  }
  if (VVERBOSE) std::cout << "Time order filtering gives " << fEdges2.size() <<
                   " edges" << std::endl;
  if (VVERBOSE) std::cout << "Phase one results:" << std::endl;
  for (size_t i = 0; i < fEdges2.size(); i++) {
    if (VVERBOSE) std::cout << edgeList.at(fEdges2.at(i)).u << " " <<
                     edgeList.at(fEdges2.at(i)).v << " " <<
                     edgeList.at(fEdges2.at(i)).time << std::endl;
  }
  return fEdges2;
}

void SearchEng::searchPhaseTwo(Task& task, std::vector<size_t> fEdges) {
  if (VVERBOSE) std::cout << "Beginning search phase two" << std::endl;
  // Fetch full edge data
  std::vector<Edge> fEdgesData;
  for (size_t i = 0; i < fEdges.size(); i++) {
    fEdgesData.push_back(edgeList.at(fEdges.at(i)));
    cycles += ADD_LATENCY + CACHE_EXP*3;
  }
  for (size_t i = 0; i < fEdgesData.size(); i++) {
    if (fEdgesData.at(i).time <= task.time) {
      if ((task.isMapped(fEdgesData.at(i).u, task.uM)
           || !task.hasMap(fEdgesData.at(i).u))
          && (task.isMapped(fEdgesData.at(i).v, task.vM)
              || !task.hasMap(fEdgesData.at(i).v))) {
        task.eG = fEdges.at(i);
        task.type = bookkeep;
        if (VVERBOSE) std::cout << "Edge match found" << std::endl;
        return;
      }
    }
    cycles += JMP_LATENCY*5 + MOV_LATENCY*2;
  }
  if (VVERBOSE) std::cout << "Edge match not found" << std::endl;
  task.eG = edgeList.size();
  task.type = backtrack;
  return;
}

void ComputeUnit::executeRootTask(Task t) {
  bool working = true;
  sEng.root_eG = t.eG;
  while (working) {
    MgrStatus mStatus;
    if (VVERBOSE) std::cout << "Updating context" << std::endl;
    cycles += TASK_LATENCY;
    mStatus = cMgr.updateContext(t);
    switch (mStatus) {
      case end:
        if (VVERBOSE) std::cout << "Manager status: end" << std::endl;
        working = false;
        break;
      case dispatch:
        if (VVERBOSE) std::cout << "Manager status: dispatch" << std::endl;
        disp.dispatch(t);
        if (VERBOSE) std::cout << "Beginning search" << std::endl;
        cycles += TASK_LATENCY;
        sEng.searchPhaseTwo(t, sEng.searchPhaseOne(t));
        break;
      case remanage:
        if (VVERBOSE) std::cout << "Manager status: remanage" << std::endl;
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
    cMems.push_back(new ContextMem());
    cUnits.push_back(new ComputeUnit(results, tM, edgeList, *(cMems.back())));
    cUnits.back()->cMgr.motifSize = tM.motif.size();
    cUnits.back()->cMgr.motifTime = tM.time;
  }
  tQ.setup(edgeList, tM.motif);  
}

void Mint::printResults() {
  std::cout << "Results:" << std::endl;
  for (size_t i = 0; i < results.store.size(); i++) {
    for (size_t j = 0; j < results.store.at(i).size(); j++) {
      std::cout << results.store.at(i).at(j).mNode << " " <<
                       results.store.at(i).at(j).gNode << " " <<
                       results.store.at(i).at(j).count << std::endl;
    }
    std::cout << "--------------------" << std::endl;
  }
  return;
}

void Mint::run() {
#pragma omp parallel
  {
    #pragma omp single
    {
      while (!tQ.tasks.empty()) {
        int nextCU = 0;
        size_t minCycles = (size_t)-1;
        if (FULL_ASYNC) {
          // Find CU that is earliest in time to give a task to
          for (size_t i = 0; i < NUM_CUS; i++) {
            if (cUnits.at(i)->cycles < minCycles) {
              nextCU = i;
              minCycles = cUnits.at(i)->cycles;
            }
          }
        } else {
          // Do static assignment like in the paper
          nextCU = tQ.tasks.front().eG % NUM_CUS;
          minCycles = cUnits.at(nextCU)->cycles;
        }
        Task nextTask = tQ.tasks.front();
        if (VERBOSE) std::cout << "Executing root task " << nextTask.eG << " with CU " <<
                         nextCU << " at cycle " << minCycles << std::endl;
#pragma omp task depend(inout: cUnits.at(nextCU)) firstprivate(nextTask, tQ, tM, edgeList) shared(results)
        {
          cUnits.at(nextCU)->cycles += DEQUEUE_LATENCY;
          cUnits.at(nextCU)->executeRootTask(nextTask);
        }
        tQ.tasks.pop();
        if (VERBOSE) std::cout << "There are " << results.store.size() <<
                         " results" << std::endl;
        if (VERBOSE) std::cout << "----------------------------------------"
                               << std::endl;
      }
    }
  } // implied taskwait
  // Collect cycle stats
  size_t maxCycles = 0;
  size_t totalCycles = 0;
  for (size_t i = 0; i < NUM_CUS; i++) {
    if (cUnits.at(i)->cycles > maxCycles) {
      maxCycles = cUnits.at(i)->cycles;
    }
    totalCycles += cUnits.at(i)->cycles;
  }
  if (VERBOSE) printResults();
  std::cout << "Total cycles taken: " << totalCycles << std::endl;
  std::cout << "End-to-end cycle count: " << maxCycles << std::endl;
  std::cout << "There are " << results.store.size() << " results" << std::endl;
  for (size_t i = 0; i < NUM_CUS; i++) {
    delete cMems.at(i);
    delete cUnits.at(i);
  }
  return;
}
