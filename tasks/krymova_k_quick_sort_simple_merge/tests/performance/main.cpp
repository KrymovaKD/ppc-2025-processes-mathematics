#include <gtest/gtest.h>
#include <mpi.h>

#include <algorithm>
#include <string>
#include <vector>

#include "krymova_k_quick_sort_simple_merge/common/include/common.hpp"
#include "krymova_k_quick_sort_simple_merge/mpi/include/ops_mpi.hpp"
#include "krymova_k_quick_sort_simple_merge/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace krymova_k_quick_sort_simple_merge {

class KrymovaKQuickSortSimpleMergePerfTests : public ppc::util::BaseRunPerfTests<InType, OutType> {
  const size_t kArraySize_ = 100000;
  InType input_data_;

  void SetUp() override {
    input_data_.resize(kArraySize_);
    for (size_t i = 0; i < kArraySize_; ++i) {
      if (i < kArraySize_ / 2) {
        input_data_[i] = static_cast<int>(kArraySize_ - i);
      } else {
        input_data_[i] = static_cast<int>((i * 37) % kArraySize_);
      }
    }
  }

  bool CheckTestOutputData(OutType &output_data) final {
    if (output_data.size() != kArraySize_) {
      return false;
    }

    return std::is_sorted(output_data.begin(), output_data.end());
  }

  InType GetTestInputData() final {
    return input_data_;
  }
};

TEST_P(KrymovaKQuickSortSimpleMergePerfTests, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, KrymovaKQuickSortSimpleMergeMPI, KrymovaKQuickSortSimpleMergeSEQ>(
        PPC_SETTINGS_krymova_k_quick_sort_simple_merge);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = KrymovaKQuickSortSimpleMergePerfTests::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, KrymovaKQuickSortSimpleMergePerfTests, kGtestValues, kPerfTestName);

}  // namespace krymova_k_quick_sort_simple_merge
