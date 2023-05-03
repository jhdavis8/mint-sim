// Mint simulator header file

#include <vector>
#include <stack>
#include <queue>
#include <bits/stdc++.h>

#define NUM_CUS 8
#define MOTIF_SIZE 5
#define VERBOSE 1

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
  void insertMapping(int gN, int mN);

  // Remove a mapping between the given nodes from the nodeMap.
  void removeMapping(int gN, int mN);
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

// *****************************************************************************
// *                         Architecture Components                           *
// *****************************************************************************

enum MgrStatus { end, dispatch, remanage };

class ContextMgr {
 public:
  ContextMem& cMem;
  MappingStore& results;
  std::vector<Edge>& edgeList;
  int& cycles;
  int motifSize;
  int motifTime;

  // Link ContextMem, edgeList, and MappingStore to ContextMgr.
  ContextMgr(ContextMem& c, MappingStore& r, std::vector<Edge>& eL, int& cyc):
      cMem(c), results(r), edgeList(eL), cycles(cyc) {}

  // Update ContextMem according to info in task. Returns a status code to
  // direct the ComputeUnit how to continue.
  MgrStatus updateContext(Task& task);
};

class Dispatcher {
 public:
  ContextMem& cMem;
  TargetMotif& tM;
  int& cycles;

  // Link Dispatcher to ContentMem and TargetMotif.
  Dispatcher(ContextMem& c, TargetMotif& m, int& cyc):
      cMem(c), tM(m), cycles(cyc) {}

  // Load necessary data into task from TargetMotif and ContextMem.
  void dispatch(Task& task);
};

class SearchEng {
 public:
  ContextMem& cMem;
  std::vector<Edge>& edgeList;
  int& cycles;

  // Link SearchEng to ContextMem.
  SearchEng(ContextMem& c, std::vector<Edge>& eL, int& cyc):
      cMem(c), edgeList(eL), cycles(cyc) {}

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
  int cycles = 0;
  ContextMem& cMem;
  ContextMgr cMgr;
  Dispatcher disp;
  SearchEng sEng;

  // Link all components appropriately.
  ComputeUnit(MappingStore& r, TargetMotif& t, std::vector<Edge>& eL,
              ContextMem& c):
      results(r), tM(t), edgeList(eL), cMem(c),
      cMgr(c, results, edgeList, cycles), disp(c, tM, cycles),
      sEng(c, edgeList, cycles) {}

  // Executes a root task to completion. Records total cycles taken. Writes
  // resulting finds to the MappingStore.
  void executeRootTask(Task t);
};

class Mint {
 public:
  std::vector<ComputeUnit> cUnits;
  std::vector<ContextMem> cMems;
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

