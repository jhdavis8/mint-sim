// Mint simulator header file

#include <vector>
#include <stack>
#include <queue>
#include <unordered_map>
#include <iostream>
#include <bits/stdc++.h>

#define NUM_CUS 512
#define MOTIF_SIZE 5
#define VERBOSE 0
#define VVERBOSE 0
#define FULL_ASYNC 0
#define DEQUEUE_LATENCY 1
#define CMEM_LATENCY 2
#define CACHE_LATENCY 2
#define DRAM_LATENCY 20
#define TASK_LATENCY 5
#define ADD_LATENCY 1
#define MUL_LATENCY 3
#define DIV_LATENCY 15
#define JMP_LATENCY 2
#define MOV_LATENCY 1
#ifndef CACHE_MISS
#define CACHE_MISS 0.20
#endif
#define CACHE_HIT (1-CACHE_MISS)
#define CACHE_EXP ((int)(DRAM_LATENCY*CACHE_MISS) + (int)(CACHE_LATENCY*CACHE_HIT))
#ifndef USE_MEMO
#define USE_MEMO 1
#endif
#ifndef MEMO_THRESH
#define MEMO_THRESH 256
#endif

// *****************************************************************************
// *                             Data Structures                               *
// *****************************************************************************

class Edge {
 public:
  int u;
  int v;
  int time;

  Edge(int uN, int vN, int t): u(uN), v(vN), time(t) {}

  Edge(): Edge(0, 0, 0) {}
};

class Mapping {
 public:
  int mNode;
  int gNode;
  int count;

  Mapping(int m, int g, int c): mNode(m), gNode(g), count(c) {}

  Mapping(): Mapping(0, 0, 0) {}
};

enum TaskType { search, bookkeep, backtrack };

class Task {
 public:
  TaskType type;
  size_t eG;
  int eM = 0;
  int uG = -1;
  int vG = -1;
  int uM = -1;
  int vM = -1;
  int time = INT_MAX;
  std::vector<Mapping> nodeMap;

  // Return true iff there exists a Mapping in nodeMap between gN and mN.
  bool isMapped(int gN, int mN);

  // Return true iff there exists a Mapping involving gN.
  bool hasMap(int gN);

  // Add a mapping between the given nodes to the nodeMap.
  void insertMapping(int gN, int mN, size_t& cycles);

  // Remove a mapping between the given nodes from the nodeMap.
  void removeMapping(int gN, int mN, size_t& cycles);

 private:
  // Print the entries of the nodeMap
  void printNodeMap();
};

class TaskQueue {
 public:
  std::queue<Task> tasks;

  // Fill TaskQueue with a root task for every node in graph.
  void setup(std::vector<Edge>& edgeList, std::vector<Edge>& motif);
};

class TargetMotif {
 public:
  std::vector<Edge> motif;
  int time;
};

class ContextMem {
 public:
  bool busy = false;
  size_t eG;
  int eM = 0;
  int uG;
  int vG;
  int uM;
  int vM;
  int time = INT_MAX;
  std::stack<int> eStack;
  std::vector<Mapping> nodeMap;
};

class MappingStore {
 public:
  std::vector<std::vector<Mapping>> store;

  // Store CAM of found motif.
  void addResult(ContextMem& cMem);
};

class Memo {
 public:
  int listIndex;
};

class MemoStruct {
 public:
  std::unordered_map<size_t, Memo> outgoing;
  std::unordered_map<size_t, Memo> incoming;
  
  // Return memoized starting index as appropriate given context
  size_t getStart(bool uCheck, bool vCheck, int uG, int vG, int eG,
                  size_t size, size_t& cycles) {
    if ((USE_MEMO && size > MEMO_THRESH) && uCheck != vCheck) {
      if (VVERBOSE) std::cout << "Checking for memo" << std::endl;
      cycles += JMP_LATENCY*2;
      if (uCheck && outgoing.find(uG) != outgoing.end()) {
        cycles += CACHE_LATENCY;
        return outgoing[uG].listIndex;
      } else if (vCheck && incoming.find(vG) != incoming.end()) {
        cycles += CACHE_LATENCY;
        return incoming[vG].listIndex;
      } else {
        return 0;
      }
    } else {
      return 0;
    }
  }

  // Memoize search index if appropriate
  void record(bool uCheck, bool vCheck, int uG, int vG, size_t root_eG,
              size_t qI, size_t i, bool& recorded, size_t size, size_t& cycles) {
    if (USE_MEMO && ((size > MEMO_THRESH && !recorded) && (uCheck != vCheck))) {
      if (VVERBOSE) std::cout << "Trying to record memo" << std::endl;
      cycles += JMP_LATENCY*2;
      if (uCheck && (outgoing.find(uG) == outgoing.end() && qI >= root_eG)) {
        outgoing[uG] = Memo();
        outgoing[uG].listIndex = i;
        recorded = true;
        cycles += DRAM_LATENCY;
        return;
      } else if (vCheck &&
                 (incoming.find(vG) == incoming.end() && qI >= root_eG)) {
        incoming[vG] = Memo();
        incoming[vG].listIndex = i;
        recorded = true;
        cycles += DRAM_LATENCY;
        return;
      }
    } else {
      return;
    }
  }
};

// *****************************************************************************
// *                         Architecture Components                           *
// *****************************************************************************

enum MgrStatus { end, dispatch, remanage };

class ContextMgr {
 public:
  ContextMem& cMem;
  MappingStore& results;
  std::vector<Edge>& edgeList;
  size_t& cycles;
  int motifSize;
  int motifTime;

  // Link ContextMem, edgeList, and MappingStore to ContextMgr.
  ContextMgr(ContextMem& c, MappingStore& r, std::vector<Edge>& eL, size_t& cyc):
      cMem(c), results(r), edgeList(eL), cycles(cyc) {}

  // Update ContextMem according to info in task. Returns a status code to
  // direct the ComputeUnit how to continue.
  MgrStatus updateContext(Task& task);
};

class Dispatcher {
 public:
  ContextMem& cMem;
  TargetMotif& tM;
  size_t& cycles;

  // Link Dispatcher to ContentMem and TargetMotif.
  Dispatcher(ContextMem& c, TargetMotif& m, size_t& cyc):
      cMem(c), tM(m), cycles(cyc) {}

  // Load necessary data into task from TargetMotif and ContextMem.
  void dispatch(Task& task);
};

class SearchEng {
 public:
  ContextMem& cMem;
  std::vector<Edge>& edgeList;
  size_t& cycles;
  MemoStruct& memo;
  int root_eG;

  // Link SearchEng to ContextMem.
  SearchEng(ContextMem& c, std::vector<Edge>& eL, size_t& cyc, MemoStruct& m):
      cMem(c), edgeList(eL), cycles(cyc), memo(m) {}

  // Linear cache-line search for successor edges
  std::vector<size_t> searchPhaseOne(Task& task);

  // Linear mapping check over filtered edges
  void searchPhaseTwo(Task& task, std::vector<size_t> fEdges);
};

class ComputeUnit {
 public:
  MappingStore& results;
  TargetMotif& tM;
  std::vector<Edge>& edgeList;
  size_t cycles = 0;
  ContextMem& cMem;
  ContextMgr cMgr;
  Dispatcher disp;
  SearchEng sEng;
  MemoStruct memo;

  // Link all components appropriately.
  ComputeUnit(MappingStore& r, TargetMotif& t, std::vector<Edge>& eL,
              ContextMem& c):
      results(r), tM(t), edgeList(eL), cMem(c),
      cMgr(c, results, edgeList, cycles), disp(c, tM, cycles),
      sEng(c, edgeList, cycles, memo) {}

  // Executes a root task to completion. Records total cycles taken. Writes
  // resulting finds to the MappingStore.
  void executeRootTask(Task t);
};

class Mint {
 public:
  std::vector<ComputeUnit*> cUnits;
  std::vector<ContextMem*> cMems;
  TaskQueue tQ;
  TargetMotif tM;
  MappingStore results;
  std::vector<Edge> edgeList;

  // Constructor
  Mint(TargetMotif m, std::vector<Edge> e);

  // Start up each ComputeUnit loop, which will draw tasks from the TaskQueue to
  // pass to ContextMgr. This continues until the TaskQueue is empty. Final
  // cycle count is the max cycles taken over each ComputeUnit.
  void run();

 private:
  void printResults();
};

