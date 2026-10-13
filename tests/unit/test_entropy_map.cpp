#include "domain/services/entropy_map.hpp"
#include <gtest/gtest.h>
#include <Eigen/Dense>

using namespace neurometrix::core2d::domain;

TEST(EntropyMap, ConstantIsZero) {
    Eigen::MatrixXf m = Eigen::MatrixXf::Constant(32, 32, 5.0f);
    const auto e = EntropyMap::compute(m, 9);
    EXPECT_LT(std::abs(e(16, 16)), 1e-3f);
}

TEST(EntropyMap, BoundedByLog2Bins) {
    Eigen::MatrixXf m = Eigen::MatrixXf::Random(32, 32);
    const auto e = EntropyMap::compute(m, 9);
    // 32 bins -> max entropy = log2(32) = 5
    EXPECT_LE(e.maxCoeff(), 5.0f + 1e-3f);
    EXPECT_GE(e.minCoeff(), -1e-6f);
}

TEST(EntropyMap, HigherDiversityHigherEntropy) {
    Eigen::MatrixXf uniform = Eigen::MatrixXf::Constant(32, 32, 50.0f);
    Eigen::MatrixXf diverse = uniform;
    for (int i = 0; i < 32; ++i)
        for (int j = 0; j < 32; ++j)
            diverse(i, j) = float((i * 32 + j) % 256);

    const auto eu = EntropyMap::compute(uniform, 9).mean();
    const auto ed = EntropyMap::compute(diverse, 9).mean();
    EXPECT_GT(ed, eu);
}