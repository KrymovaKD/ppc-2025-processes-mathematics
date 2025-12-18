#include <gtest/gtest.h>
#include <mpi.h>

#include <algorithm>
#include <random>
#include <string>
#include <tuple>
#include <vector>

#include "krymova_k_scatter/common/include/common.hpp"
#include "krymova_k_scatter/mpi/include/ops_mpi.hpp"
#include "krymova_k_scatter/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace krymova_k_scatter {

class KrymovaKScatterFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    int root = std::get<0>(test_param);
    std::string data_type = std::get<1>(test_param);
    return "root_" + std::to_string(root) + "_type_" + data_type;
  }

 protected:
  void SetUp() override {
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    original_root_ = std::get<0>(params);
    std::string data_type_str = std::get<1>(params);

    data_type_ = 0;
    if (data_type_str == "float") {
      data_type_ = 1;
    } else if (data_type_str == "double") {
      data_type_ = 2;
    }

    count_ = 100;

    int initialized = 0;
    MPI_Initialized(&initialized);
    if (initialized) {
      MPI_Comm_size(MPI_COMM_WORLD, &size_);
      MPI_Comm_rank(MPI_COMM_WORLD, &rank_);
    } else {
      size_ = 1;
      rank_ = 0;
    }

    // КОРРЕКЦИЯ: root должен быть в диапазоне [0, size_-1]
    root_ = original_root_ % size_;

    input_data_ = InType(root_, data_type_, count_);

    int total_size = size_ * count_;

    std::random_device rd;
    std::mt19937 gen(rd());

    if (data_type_ == 0) {
      std::uniform_int_distribution<int> dist(1, 1000);
      input_data_.int_data.resize(total_size);
      for (int i = 0; i < total_size; ++i) {
        input_data_.int_data[i] = dist(gen);
      }

      input_data_.float_data.clear();
      input_data_.double_data.clear();
    } else if (data_type_ == 1) {
      std::uniform_real_distribution<float> dist(0.0f, 1000.0f);
      input_data_.float_data.resize(total_size);
      for (int i = 0; i < total_size; ++i) {
        input_data_.float_data[i] = dist(gen);
      }

      input_data_.int_data.clear();
      input_data_.double_data.clear();
    } else {
      std::uniform_real_distribution<double> dist(0.0, 1000.0);
      input_data_.double_data.resize(total_size);
      for (int i = 0; i < total_size; ++i) {
        input_data_.double_data[i] = dist(gen);
      }

      input_data_.int_data.clear();
      input_data_.float_data.clear();
    }
  }

  bool CheckTestOutputData(OutType &output_data) final {
    // Для последовательной версии возвращается count_, для MPI - тоже count_
    return output_data == count_;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
  int original_root_;  // Исходный root из параметров теста
  int root_;           // Скорректированный root
  int data_type_;
  int count_;
  int size_;
  int rank_;
};

namespace {

TEST_P(KrymovaKScatterFuncTests, ScatterTest) {
  ExecuteTest(GetParam());
}

// ИСПРАВЛЕННЫЙ: убираем корни больше 2, так как они будут корректироваться
const std::array<TestType, 9> kTestParam = {
    std::make_tuple(0, "int"), std::make_tuple(0, "float"), std::make_tuple(0, "double"),
    std::make_tuple(1, "int"), std::make_tuple(1, "float"), std::make_tuple(1, "double"),
    std::make_tuple(2, "int"), std::make_tuple(2, "float"), std::make_tuple(2, "double"),
};

const auto kTestTasksList =
    std::tuple_cat(ppc::util::AddFuncTask<KrymovaKScatterMPI, InType>(kTestParam, PPC_SETTINGS_krymova_k_scatter),
                   ppc::util::AddFuncTask<KrymovaKScatterSEQ, InType>(kTestParam, PPC_SETTINGS_krymova_k_scatter));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName = KrymovaKScatterFuncTests::PrintFuncTestName<KrymovaKScatterFuncTests>;

INSTANTIATE_TEST_SUITE_P(ScatterTests, KrymovaKScatterFuncTests, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace krymova_k_scatter
