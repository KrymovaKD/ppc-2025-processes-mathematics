#pragma once

#include <string>
#include <tuple>
#include <vector>

#include "task/include/task.hpp"

namespace krymova_k_quick_sort_simple_merge {

using InType = std::vector<int>;
using OutType = std::vector<int>;
using TestType = std::tuple<int, std::string>;
using BaseTask = ppc::task::Task<InType, OutType>;

void QuickSortIterative(std::vector<int> &arr);
std::vector<int> MergeTwoSorted(const std::vector<int> &a, const std::vector<int> &b);

}  // namespace krymova_k_quick_sort_simple_merge
