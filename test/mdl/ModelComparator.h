#pragma once
#include <string>
#include <vector>
class Model;

class ModelComparator {
public:
  // Returns differences between a and b; empty means equivalent.
  static std::vector<std::string> Compare(Model *a, Model *b);
};
