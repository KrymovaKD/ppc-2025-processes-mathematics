#include "krymova_k_scatter/seq/include/ops_seq.hpp"

#include <mpi.h>

#include <cstring>
#include <vector>

#include "krymova_k_scatter/common/include/common.hpp"
#include "util/include/util.hpp"

namespace krymova_k_scatter {

KrymovaKScatterSEQ::KrymovaKScatterSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool KrymovaKScatterSEQ::ValidationImpl() {
  const auto &input = GetInput();

  if (input.root < 0) {
    return false;
  }

  int size = 1;
  int initialized = 0;
  MPI_Initialized(&initialized);
  if (initialized) {
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if (input.root >= size) {
      return false;
    }
  }

  if (input.data_type < 0 || input.data_type > 2) {
    return false;
  }
  if (input.count <= 0) {
    return false;
  }

  const int total_size = size * input.count;

  if (input.data_type == 0) {
    if (static_cast<int>(input.int_data.size()) < total_size) {
      return false;
    }
  } else if (input.data_type == 1) {
    if (static_cast<int>(input.float_data.size()) < total_size) {
      return false;
    }
  } else {
    if (static_cast<int>(input.double_data.size()) < total_size) {
      return false;
    }
  }

  return GetOutput() == 0;
}

bool KrymovaKScatterSEQ::PreProcessingImpl() {
  GetOutput() = 0;
  return true;
}

bool KrymovaKScatterSEQ::RunImpl() {
  const auto &input = GetInput();
  int rank = 0;
  int size = 1;

  int initialized = 0;
  MPI_Initialized(&initialized);
  if (initialized) {
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
  }

  MPI_Datatype mpi_type;
  switch (input.data_type) {
    case 0:
      mpi_type = MPI_INT;
      break;
    case 1:
      mpi_type = MPI_FLOAT;
      break;
    case 2:
      mpi_type = MPI_DOUBLE;
      break;
    default:
      return false;
  }

  void *sendbuf = nullptr;
  void *recvbuf = nullptr;
  const int recvcount = input.count;

  std::vector<int> recv_int;
  std::vector<float> recv_float;
  std::vector<double> recv_double;

  switch (input.data_type) {
    case 0:
      recv_int.resize(recvcount);
      recvbuf = recv_int.data();
      if (rank == input.root) {
        sendbuf = static_cast<void *>(const_cast<int *>(input.int_data.data()));
      }
      break;
    case 1:
      recv_float.resize(recvcount);
      recvbuf = recv_float.data();
      if (rank == input.root) {
        sendbuf = static_cast<void *>(const_cast<float *>(input.float_data.data()));
      }
      break;
    case 2:
      recv_double.resize(recvcount);
      recvbuf = recv_double.data();
      if (rank == input.root) {
        sendbuf = static_cast<void *>(const_cast<double *>(input.double_data.data()));
      }
      break;
  }

  int result = MPI_SUCCESS;
  if (initialized) {
    result = MPI_Scatter(sendbuf, input.count, mpi_type, recvbuf, recvcount, mpi_type, input.root, MPI_COMM_WORLD);
  } else {
    if (rank == input.root) {
      const char *base = nullptr;
      std::size_t type_size = 0;
      switch (input.data_type) {
        case 0:
          base = reinterpret_cast<const char *>(input.int_data.data());
          type_size = sizeof(int);
          break;
        case 1:
          base = reinterpret_cast<const char *>(input.float_data.data());
          type_size = sizeof(float);
          break;
        case 2:
          base = reinterpret_cast<const char *>(input.double_data.data());
          type_size = sizeof(double);
          break;
      }
      std::memcpy(recvbuf, base, recvcount * type_size);
    }
  }

  if (result != MPI_SUCCESS) {
    return false;
  }

  bool data_valid = false;
  switch (input.data_type) {
    case 0:
      data_valid = !recv_int.empty();
      break;
    case 1:
      data_valid = !recv_float.empty();
      break;
    case 2:
      data_valid = !recv_double.empty();
      break;
  }

  GetOutput() = data_valid ? recvcount : 0;

  return data_valid;
}

bool KrymovaKScatterSEQ::PostProcessingImpl() {
  return GetOutput() == GetInput().count;
}

}  // namespace krymova_k_scatter
