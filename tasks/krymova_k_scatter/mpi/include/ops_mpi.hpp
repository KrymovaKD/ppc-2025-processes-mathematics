#pragma once

#include <mpi.h>

#include "krymova_k_scatter/common/include/common.hpp"
#include "task/include/task.hpp"
namespace krymova_k_scatter {

int MyMPI_Scatter(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount,
                  MPI_Datatype recvtype, int root, MPI_Comm comm);

class KrymovaKScatterMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit KrymovaKScatterMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace krymova_k_scatter
