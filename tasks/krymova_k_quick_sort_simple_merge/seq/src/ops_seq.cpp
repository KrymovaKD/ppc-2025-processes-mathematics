#include "krymova_k_quick_sort_simple_merge/seq/include/ops_seq.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

#include "krymova_k_quick_sort_simple_merge/common/include/common.hpp"

namespace krymova_k_quick_sort_simple_merge {

namespace {

struct StackItem {
  int left;
  int right;
};

bool ShouldContinue(int left, int right) {
  return left >= right;
}

void SortThreeElements(std::vector<int> &arr, int left, int mid, int right) {
  if (arr[left] > arr[mid]) {
    std::swap(arr[left], arr[mid]);
  }
  if (arr[left] > arr[right]) {
    std::swap(arr[left], arr[right]);
  }
  if (arr[mid] > arr[right]) {
    std::swap(arr[mid], arr[right]);
  }
}

int Partition(std::vector<int> &arr, int left, int right, int pivot) {
  int i = left - 1;
  for (int j = left; j < right; ++j) {
    if (arr[j] <= pivot) {
      ++i;
      std::swap(arr[i], arr[j]);
    }
  }
  std::swap(arr[i + 1], arr[right]);
  return i + 1;
}

void AddToStack(std::vector<StackItem> &stack, int left, int right, int partition) {
  if (partition - left > right - partition) {
    stack.push_back({left, partition - 1});
    stack.push_back({partition + 1, right});
  } else {
    stack.push_back({partition + 1, right});
    stack.push_back({left, partition - 1});
  }
}

}  // namespace

void QuickSortIterative(std::vector<int> &arr) {
  if (arr.size() <= 1) {
    return;
  }

  std::vector<StackItem> stack{{0, static_cast<int>(arr.size()) - 1}};

  while (!stack.empty()) {
    const auto [left, right] = stack.back();
    stack.pop_back();

    if (ShouldContinue(left, right)) {
      continue;
    }

    const int mid = left + ((right - left) / 2);

    SortThreeElements(arr, left, mid, right);
    std::swap(arr[mid], arr[right]);
    const int pivot = arr[right];

    const int partition = Partition(arr, left, right, pivot);
    AddToStack(stack, left, right, partition);
  }
}

std::vector<int> MergeTwoSorted(const std::vector<int> &a, const std::vector<int> &b) {
  std::vector<int> result;
  result.reserve(a.size() + b.size());

  size_t i = 0;
  size_t j = 0;

  while (i < a.size() && j < b.size()) {
    if (a[i] <= b[j]) {
      result.push_back(a[i++]);
    } else {
      result.push_back(b[j++]);
    }
  }

  result.insert(result.end(), a.begin() + static_cast<int>(i), a.end());
  result.insert(result.end(), b.begin() + static_cast<int>(j), b.end());

  return result;
}

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
