#include "krymova_k_quick_sort_simple_merge/seq/include/ops_seq.hpp"

#include <algorithm>
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

void QuickSortIterative(std::vector<int> &arr) {
  if (arr.size() <= 1) {
    return;
  }

  struct StackItem {
    int left;
    int right;
  };
  std::vector<StackItem> stack{{0, static_cast<int>(arr.size()) - 1}};

  while (!stack.empty()) {
    const auto [left, right] = stack.back();
    stack.pop_back();

    if (left >= right) {
      continue;
    }

    const int mid = left + ((right - left) / 2);

    if (arr[left] > arr[mid]) {
      std::swap(arr[left], arr[mid]);
    }
    if (arr[left] > arr[right]) {
      std::swap(arr[left], arr[right]);
    }
    if (arr[mid] > arr[right]) {
      std::swap(arr[mid], arr[right]);
    }

    std::swap(arr[mid], arr[right]);
    const int pivot = arr[right];

    int i = left - 1;
    for (int j = left; j < right; ++j) {
      if (arr[j] <= pivot) {
        ++i;
        std::swap(arr[i], arr[j]);
      }
    }

    std::swap(arr[i + 1], arr[right]);
    const int partition = i + 1;

    if (partition - left > right - partition) {
      stack.push_back({left, partition - 1});
      stack.push_back({partition + 1, right});
    } else {
      stack.push_back({partition + 1, right});
      stack.push_back({left, partition - 1});
    }
  }
}

std::vector<int> MergeTwoSorted(const std::vector<int> &a, const std::vector<int> &b) {
  std::vector<int> result;
  result.reserve(a.size() + b.size());

  std::size_t i = 0;
  std::size_t j = 0;

  while (i < a.size() && j < b.size()) {
    if (a[i] <= b[j]) {
      result.push_back(a[i++]);
    } else {
      result.push_back(b[j++]);
    }
  }

  result.insert(result.end(), a.begin() + static_cast<std::ptrdiff_t>(i), a.end());
  result.insert(result.end(), b.begin() + static_cast<std::ptrdiff_t>(j), b.end());

  return result;
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
