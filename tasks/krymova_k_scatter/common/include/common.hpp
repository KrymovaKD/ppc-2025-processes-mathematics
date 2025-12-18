#pragma once

#include <string>
#include <tuple>
#include <vector>

#include "task/include/task.hpp"

namespace krymova_k_scatter {
struct ScatterInput {
  int root;
  int data_type;
  int count;
  std::vector<int> int_data;
  std::vector<float> float_data;
  std::vector<double> double_data;

  ScatterInput() : root(0), data_type(0), count(0) {}
  ScatterInput(int r, int dt, int c) : root(r), data_type(dt), count(c) {}
};

using InType = ScatterInput;
using OutType = int;
using TestType = std::tuple<int, std::string>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace krymova_k_scatter
