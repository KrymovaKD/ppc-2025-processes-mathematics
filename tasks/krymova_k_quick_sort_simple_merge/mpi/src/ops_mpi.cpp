#include "krymova_k_quick_sort_simple_merge/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <iostream>
#include <queue>
#include <vector>

namespace krymova_k_quick_sort_simple_merge {

KrymovaKQuickSortSimpleMergeMPI::KrymovaKQuickSortSimpleMergeMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = std::vector<int>();
}

bool KrymovaKQuickSortSimpleMergeMPI::ValidationImpl() {
  return true;
}

bool KrymovaKQuickSortSimpleMergeMPI::PreProcessingImpl() {
  return true;
}

void KrymovaKQuickSortSimpleMergeMPI::quickSortIterative(std::vector<int> &arr) {
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

    int mid = left + (right - left) / 2;
    int pivot_idx;

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
std::vector<int> KrymovaKQuickSortSimpleMergeMPI::mergeTwoSorted(const std::vector<int> &a, const std::vector<int> &b) {
  std::vector<int> result;
  result.reserve(a.size() + b.size());

  size_t i = 0, j = 0;

  while (i < a.size() && j < b.size()) {
    if (a[i] <= b[j]) {
      result.push_back(a[i++]);
    } else {
      result.push_back(b[j++]);
    }
  }

  result.insert(result.end(), a.begin() + i, a.end());
  result.insert(result.end(), b.begin() + j, b.end());

  return result;
}

bool KrymovaKQuickSortSimpleMergeMPI::RunImpl() {
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  std::vector<int> all_data;
  int total_size = 0;

  if (rank == 0) {
    all_data = GetInput();
    total_size = static_cast<int>(all_data.size());
  }

  MPI_Bcast(&total_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (total_size == 0) {
    GetOutput() = std::vector<int>();
    return true;
  }

  int base_chunk = total_size / size;
  int remainder = total_size % size;

  std::vector<int> send_counts(size, base_chunk);
  std::vector<int> send_displs(size, 0);

  for (int i = 0; i < remainder; ++i) {
    send_counts[i]++;
  }

  for (int i = 1; i < size; ++i) {
    send_displs[i] = send_displs[i - 1] + send_counts[i - 1];
  }

  int local_size = send_counts[rank];
  std::vector<int> local_data(local_size);

  if (total_size > 0) {
    MPI_Scatterv(all_data.data(), send_counts.data(), send_displs.data(), MPI_INT, local_data.data(), local_size,
                 MPI_INT, 0, MPI_COMM_WORLD);
  }

  quickSortIterative(local_data);

  std::vector<int> current_data = local_data;
  int partner_distance = 1;

  while (partner_distance < size) {
    int partner_rank = rank ^ partner_distance;

    if (partner_rank < size) {
      int my_size = static_cast<int>(current_data.size());
      int partner_size;

      MPI_Sendrecv(&my_size, 1, MPI_INT, partner_rank, 0, &partner_size, 1, MPI_INT, partner_rank, 0, MPI_COMM_WORLD,
                   MPI_STATUS_IGNORE);

      std::vector<int> partner_data(partner_size);
      if (my_size > 0 && partner_size > 0) {
        MPI_Sendrecv(current_data.data(), my_size, MPI_INT, partner_rank, 1, partner_data.data(), partner_size, MPI_INT,
                     partner_rank, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      } else if (my_size > 0) {
        MPI_Send(current_data.data(), my_size, MPI_INT, partner_rank, 1, MPI_COMM_WORLD);
      } else if (partner_size > 0) {
        MPI_Recv(partner_data.data(), partner_size, MPI_INT, partner_rank, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      }

      if (rank < partner_rank) {
        current_data = mergeTwoSorted(current_data, partner_data);
      } else {
        current_data.clear();
      }
    }

    partner_distance <<= 1;
    MPI_Barrier(MPI_COMM_WORLD);
  }

  std::vector<int> final_result;

  if (rank == 0) {
    final_result = current_data;

    for (int i = 1; i < size; ++i) {
      int part_size;
      MPI_Recv(&part_size, 1, MPI_INT, i, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      if (part_size > 0) {
        std::vector<int> part_data(part_size);
        MPI_Recv(part_data.data(), part_size, MPI_INT, i, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        final_result = mergeTwoSorted(final_result, part_data);
      }
    }
  } else {
    int my_size = static_cast<int>(current_data.size());
    MPI_Send(&my_size, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);

    if (my_size > 0) {
      MPI_Send(current_data.data(), my_size, MPI_INT, 0, 3, MPI_COMM_WORLD);
    }
  }

  int final_size = 0;
  if (rank == 0) {
    final_size = static_cast<int>(final_result.size());
  }

  MPI_Bcast(&final_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (rank != 0) {
    final_result.resize(final_size);
  }

  if (final_size > 0) {
    MPI_Bcast(final_result.data(), final_size, MPI_INT, 0, MPI_COMM_WORLD);
  }

  GetOutput() = final_result;
  return true;
}

bool KrymovaKQuickSortSimpleMergeMPI::PostProcessingImpl() {
  return true;
}

}  // namespace krymova_k_quick_sort_simple_merge
