#include "krymova_k_quick_sort_simple_merge/seq/include/ops_seq.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

#include "krymova_k_quick_sort_simple_merge/common/include/common.hpp"

namespace krymova_k_quick_sort_simple_merge {

KrymovaKQuickSortSimpleMergeSEQ::KrymovaKQuickSortSimpleMergeSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = std::vector<int>();
}

bool KrymovaKQuickSortSimpleMergeSEQ::ValidationImpl() {
  return true;
}

bool KrymovaKQuickSortSimpleMergeSEQ::PreProcessingImpl() {
  return true;
}

void KrymovaKQuickSortSimpleMergeSEQ::QuickSortIterative(std::vector<int> &arr) {
  if (arr.size() <= 1) {
    return;
  }

  struct StackItem {
    int left;
    int right;
  };

  std::vector<StackItem> stack;
  stack.push_back({0, static_cast<int>(arr.size()) - 1});

  while (!stack.empty()) {
    auto [left, right] = stack.back();
    stack.pop_back();

    if (left >= right) {
      continue;
    }

    int mid = left + ((right - left) / 2);
    int pivot_idx = mid;
    if (arr[left] > arr[mid]) {
      if (arr[mid] > arr[right]) {
        pivot_idx = mid;
      } else if (arr[left] > arr[right]) {
        pivot_idx = right;
      } else {
        pivot_idx = left;
      }
    } else {
      if (arr[left] > arr[right]) {
        pivot_idx = left;
      } else if (arr[mid] > arr[right]) {
        pivot_idx = right;
      } else {
        pivot_idx = mid;
      }
    }

    std::swap(arr[pivot_idx], arr[right]);
    int pivot_value = arr[right];

    int i = left - 1;
    for (int j = left; j < right; j++) {
      if (arr[j] <= pivot_value) {
        i++;
        std::swap(arr[i], arr[j]);
      }
    }

    std::swap(arr[i + 1], arr[right]);
    int partition = i + 1;

    if (partition - left > right - partition) {
      stack.push_back({left, partition - 1});
      stack.push_back({partition + 1, right});
    } else {
      stack.push_back({partition + 1, right});
      stack.push_back({left, partition - 1});
    }
  }
}

bool KrymovaKQuickSortSimpleMergeSEQ::RunImpl() {
  std::vector<int> data = GetInput();
  QuickSortIterative(data);
  GetOutput() = data;
  return true;
}

bool KrymovaKQuickSortSimpleMergeSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace krymova_k_quick_sort_simple_merge
