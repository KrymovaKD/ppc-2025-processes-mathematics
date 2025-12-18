#include "krymova_k_scatter/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <cstring>
#include <vector>

#include "krymova_k_scatter/common/include/common.hpp"
#include "util/include/util.hpp"

namespace krymova_k_scatter {

KrymovaKScatterMPI::KrymovaKScatterMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool KrymovaKScatterMPI::ValidationImpl() {
  int size = 1;
  int initialized = 0;
  MPI_Initialized(&initialized);
  if (initialized) {
    MPI_Comm_size(MPI_COMM_WORLD, &size);
  }

  const auto &input = GetInput();
  if (input.root < 0 || input.root >= size) {
    return false;
  }
  if (input.data_type < 0 || input.data_type > 2) {
    return false;
  }
  if (input.count <= 0) {
    return false;
  }
  const int total_size = size * input.count;
  if (input.data_type == 0 && static_cast<int>(input.int_data.size()) < total_size) {
    return false;
  }
  if (input.data_type == 1 && static_cast<int>(input.float_data.size()) < total_size) {
    return false;
  }
  if (input.data_type == 2 && static_cast<int>(input.double_data.size()) < total_size) {
    return false;
  }
  return GetOutput() == 0;
}

bool KrymovaKScatterMPI::PreProcessingImpl() {
  GetOutput() = 0;
  return true;
}

int MyMPI_Scatter(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount,
                  MPI_Datatype recvtype, int root, MPI_Comm comm) {
  (void)recvtype;  // Явно указываем, что параметр не используется

  int rank = 0;
  int size = 0;
  MPI_Comm_rank(comm, &rank);
  MPI_Comm_size(comm, &size);

  if (sendcount != recvcount) {
    return MPI_ERR_COUNT;
  }

  int type_size = 0;
  if (MPI_Type_size(sendtype, &type_size) != MPI_SUCCESS) {
    return MPI_ERR_TYPE;
  }

  if (size == 1) {
    if (rank == root && sendbuf != MPI_IN_PLACE) {
      std::memcpy(recvbuf, sendbuf, static_cast<std::size_t>(sendcount) * type_size);
    }
    return MPI_SUCCESS;
  }

  const char *base = nullptr;
  if (sendbuf == MPI_IN_PLACE) {
    if (rank == root) {
      base = reinterpret_cast<const char *>(recvbuf);
    }
  } else {
    base = reinterpret_cast<const char *>(sendbuf);
  }

  const int virtual_rank = (rank - root + size) % size;

  int mask = 1;
  while (mask < size) {
    if ((virtual_rank & mask) == 0) {
      const int child_virtual = virtual_rank | mask;
      if (child_virtual < size) {
        const int child_real = (child_virtual + root) % size;

        if (rank == root && base != nullptr) {
          const char *child_data = base + child_real * sendcount * type_size;
          MPI_Send(child_data, sendcount, sendtype, child_real, 0, comm);
        } else if (rank != child_real) {
          MPI_Send(recvbuf, sendcount, sendtype, child_real, 0, comm);
        }
      }
    } else {
      const int parent_virtual = virtual_rank & ~mask;
      const int parent_real = (parent_virtual + root) % size;
      MPI_Recv(recvbuf, recvcount, sendtype, parent_real, 0, comm, MPI_STATUS_IGNORE);
    }
    mask <<= 1;
  }

  if (rank == root && sendbuf != MPI_IN_PLACE && base != nullptr) {
    const char *my_data = base + rank * sendcount * type_size;
    std::memcpy(recvbuf, my_data, static_cast<std::size_t>(sendcount) * type_size);
  }

  return MPI_SUCCESS;
}

bool KrymovaKScatterMPI::RunImpl() {
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
    result = MyMPI_Scatter(sendbuf, input.count, mpi_type, recvbuf, recvcount, mpi_type, input.root, MPI_COMM_WORLD);
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

  if (initialized) {
    MPI_Barrier(MPI_COMM_WORLD);
  }

  return data_valid;
}

bool KrymovaKScatterMPI::PostProcessingImpl() {
  return GetOutput() == GetInput().count;
}

}  // namespace krymova_k_scatter
