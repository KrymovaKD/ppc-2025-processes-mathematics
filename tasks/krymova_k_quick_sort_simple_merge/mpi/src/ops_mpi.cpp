#include "krymova_k_quick_sort_simple_merge/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <cstddef>
#include <vector>

#include "krymova_k_quick_sort_simple_merge/common/include/common.hpp"

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
bool KrymovaKQuickSortSimpleMergeMPI::RunImpl() {
  int rank = 0;
  int size = 0;
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

  QuickSortIterative(local_data);

  std::vector<int> current_data = local_data;
  int partner_distance = 1;

  while (partner_distance < size) {
    int partner_rank = rank ^ partner_distance;

    if (partner_rank < size) {
      int my_size = static_cast<int>(current_data.size());
      int partner_size = 0;

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
        current_data = MergeTwoSorted(current_data, partner_data);
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
      int part_size = 0;
      MPI_Recv(&part_size, 1, MPI_INT, i, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      if (part_size > 0) {
        std::vector<int> part_data(part_size);
        MPI_Recv(part_data.data(), part_size, MPI_INT, i, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        final_result = MergeTwoSorted(final_result, part_data);
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
