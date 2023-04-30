// Mint simulator header file

#include <array>
#include <vector>
#include <stack>
#include <queue>
#include <bits/stdc++.h>

#define NUM_CUS 8
#define MOTIF_SIZE 5

// *****************************************************************************
// *                             Data Structures                               *
// *****************************************************************************

class Edge {
 public:
  int u;
  int v;
  int time;
};

class Mapping {
 public:
  int mNode;
  int gNode;
  int count;
};

enum TaskType { search, bookkeep, backtrack };

class Task {
 public:
  TaskType type;
  int eG = -1;
  int eM = -1;
  int uG = -1;
  int vG = -1;
  int uM = -1;
  int vM = -1;
  int time = INT_MAX;
};

class TaskQueue {
 public:
  std::queue<Task> tasks;

  // Fill TaskQueue with a root task for every node in graph.
  void loadTasks();
};

class TargetMotif {
 public:
  std::array<Edge, MOTIF_SIZE> motif;

  // Load list of Edges for the target motif.
  void loadMotif();
};

class MappingStore {
 public:
  std::vector<std::vector<Mapping>> store;

  // Store CAM of found motif.
  void addResult(ContextMem& cMem);
};

class ContextMem {
 public:
  bool busy = false;
  int eG = -1;
  int eM = -1;
  int time = INT_MAX;
  std::stack<int> eStack;
  std::vector<Mapping> nodeMap;
};

// *****************************************************************************
// *                         Architecture Components                           *
// *****************************************************************************

enum MgrStatus { end, dispatch, backtrack };

class ContextMgr {
 public:
  ContextMem& cMem;
  MappingStore& mStr;
  std::vector<Edge>& edgeList;
  int& cycles;

  // Link ContextMem, edgeList, and MappingStore to ContextMgr.
  void setup(ContextMem& c, MappingStore& m, std::vector<Edge>& eL, int& cyc);

  // Update ContextMem according to info in task. Returns a status code to
  // direct the ComputeUnit how to continue.
  MgrStatus updateContext(Task& task);
};

class Dispatcher {
 public:
  ContextMem& cMem;
  TargetMotif& motif;
  int& cycles;

  // Link Dispatcher to ContentMem and TargetMotif.
  void setup(ContextMem& c, TargetMotif& m, int& cyc);

  // Load necessary data into task from TargetMotif and ContextMem.
  void dispatch(Task& task);
};

class SearchEng {
 public:
  ContextMem& cMem;
  std::vector<Edge>& edgeList;
  int& cycles;

  // Link SearchEng to ContextMem.
  void setup(ContextMem& c, std::vector<Edge>& eL, int& cyc);

  // Linear cache-line search for successor edges
  std::vector<Edge> searchPhaseOne(Task& task);

  // Linear mapping check over filtered edges
  void searchPhaseTwo(Task& task, std::vector<Edge> filteredEdges);
};

class ComputeUnit {
 public:
  ContextMem cMem;
  ContextMgr cMgr;
  Dispatcher disp;
  SearchEng sEng;
  int cycles;

  // Allocate memory for all components and link them appropriately.
  void setup();
  
  // Executes a root task to completion. Records total cycles taken. Writes
  // resulting finds to the MappingStore.
  void executeRootTask(Task t, MappingStore& results);
};

class Mint {
 public:
  std::array<ComputeUnit, NUM_CUS> cUnits;
  TaskQueue tQ;
  TargetMotif tM;
  MappingStore results;
  std::vector<Edge> edgeList;
  
  // Load motif into TargetMotif and load graph into DRAM, set up TaskQueue.
  // Call setup method for each ComputeUnit.
  void loadData();

  // Start up each ComputeUnit loop, which will draw tasks from the TaskQueue to
  // pass to ContextMgr. This continues until the TaskQueue is empty. Final
  // cycle count is the max cycles taken over each ComputeUnit.
  void run();
};

