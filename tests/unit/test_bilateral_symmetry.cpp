#include "domain/services/bilateral_symmetry.hpp"
#include <gtest/gtest.h>
#include <Eigen/Dense>

using namespace neurometrix::core2d::domain;

TEST(BilateralSymmetry, PerfectSymmetryScoresZero) {
    Eigen::MatrixXf m = Eigen::MatrixXf::Zero(32, 32);
    for (int i = 0; i < 32; ++i)
        for (int j = 0; j < 16; ++j) {
            const float v = 40.0f + 0.5f * i + 0.3f * j;
            m(i, j) = v;
            m(i, 31 - j) = v;
        }
    const auto r = BilateralSymmetry::analyze(m);
    EXPECT_LT(r.asymmetry_score, 1e-4);
    EXPECT_LT(r.peak_value, 1e-3);
}

TEST(BilateralSymmetry, UnilateralLesionDetected) {
    Eigen::MatrixXf m = Eigen::MatrixXf::Zero(64, 64);
    for (int i = 28; i < 36; ++i)
        for (int j = 12; j < 20; ++j)
            m(i, j) = 80.0f;
    const auto r = BilateralSymmetry::analyze(m);
    EXPECT_GT(r.asymmetry_score, 0.5);
    EXPECT_GT(r.peak_value, 60.0);
    EXPECT_GE(r.peak_row, 26);
    EXPECT_LE(r.peak_row, 38);
    EXPECT_GE(r.peak_col, 10);
    EXPECT_LE(r.peak_col, 22);
}

TEST(BilateralSymmetry, ScoreMonotoneWithLesionSize) {
    auto score_for = [](int size) {
        Eigen::MatrixXf m = Eigen::MatrixXf::Zero(64, 64);
        for (int i = 32 - size; i < 32 + size; ++i)
            for (int j = 16 - size; j < 16 + size; ++j)
                m(i, j) = 60.0f;
        return BilateralSymmetry::analyze(m).asymmetry_score;
    };
    EXPECT_LT(score_for(2), score_for(4));
    EXPECT_LT(score_for(4), score_for(6));
}