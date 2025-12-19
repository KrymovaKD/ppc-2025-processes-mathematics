#include <gtest/gtest.h>
#include <mpi.h>

#include <random>
#include <vector>

#include "krymova_k_scatter/common/include/common.hpp"
#include "krymova_k_scatter/mpi/include/ops_mpi.hpp"
#include "krymova_k_scatter/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace krymova_k_scatter {

class KrymovaKScatterPerfTests : public ppc::util::BaseRunPerfTests<InType, OutType> {
 protected:
  void SetUp() override {
    count_ = 5000000;

    MPI_Comm_size(MPI_COMM_WORLD, &size_);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank_);

    int root = size_ - 1;

    input_data_ = InType(root, 0, count_);

    int total_size = size_ * count_;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(1, 1000);

    input_data_.int_data.resize(total_size);
    for (int i = 0; i < total_size; ++i) {
      input_data_.int_data[i] = dist(gen);
    }

    // Очищаем ненужные буферы
    input_data_.float_data.clear();
    input_data_.double_data.clear();
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return output_data == count_;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_{};
  int size_;
  int rank_;
  int count_;
};

TEST_P(KrymovaKScatterPerfTests, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, KrymovaKScatterMPI, KrymovaKScatterSEQ>(PPC_SETTINGS_krymova_k_scatter);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = KrymovaKScatterPerfTests::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, KrymovaKScatterPerfTests, kGtestValues, kPerfTestName);

}  // namespace krymova_k_scatter
