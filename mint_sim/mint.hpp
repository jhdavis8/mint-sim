// Mint simulator header file

#include <array>
#include <vector>
#include <stack>
#include <queue>
#include <bits/stdc++.h>

#define NUM_CUS 8
#define MOTIF_SIZE 5

enum TaskType { search, bookkeep, backtrack };

class Edge {
 public:
  int u;
  int v;
  int time;
};

class Mapping {
 public:
  
}

class CAM {
 public:
  std::vector<Mapping> data;
};

class ContextMem {
 public:
  bool busy = false;
  int e_G = -1;
  int e_M = -1;
  int time = INT_MAX;
  std::stack<int> eStack;
  CAM nodeMap;
};

class Task {
 public:
  TaskType type;
  int e_G = -1;
  int e_M = -1;
  int time = INT_MAX;
  std::stack<int> eStack;
  CAM nodeMap;
};


class ContextManager {
 public:
  Task inFlight;
};

class Dispatcher {
 public:
  void dispatch();
};

class SearchEngine {
 public:
  Task inFlight;
};

class ComputeUnit {
 public:
  ContextMem cMem;
  ContextManager cMan;
  Dispatcher disp;
  SearchEngine sEng;
};

class TaskQueue {
 public:
  std::queue<Task> tasks;
};

class TargetMotif {
 public:
  std::array<Edge, MOTIF_SIZE> motif;
};

class Mint {
 public:
  std::array<ComputeUnit, NUM_CUS> cUnits;
  TaskQueue tQ;
  TargetMotif tM;
};

