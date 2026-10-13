#include "domain/services/physics_filters.hpp"
#include <gtest/gtest.h>
#include <Eigen/Dense>

using namespace neurometrix::core2d::domain;

TEST(PhysicsFilters, TVPreservesConstantExactly) {
    for (int n : {8, 16, 32, 64}) {
        Eigen::MatrixXf m = Eigen::MatrixXf::Constant(n, n, 42.7f);
        const auto out = PhysicsFilters::total_variation_denoise(m, 0.1f, 20);
        EXPECT_LT((out - m).cwiseAbs().maxCoeff(), 1e-4f);
    }
}

TEST(PhysicsFilters, TVReducesNoise) {
    const int N = 48;
    Eigen::MatrixXf clean = Eigen::MatrixXf::Constant(N, N, 100.0f);
    Eigen::MatrixXf noisy = clean;
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            noisy(i,j) += 15.0f * std::sin(1.3f*i + 0.7f*j);

    const float before = (noisy - clean).squaredNorm();
    const auto d = PhysicsFilters::total_variation_denoise(noisy, 0.5f, 100);
    const float after = (d - clean).squaredNorm();
    EXPECT_LT(after, before * 0.5f) << "TV must reduce noise by >50%";
}

TEST(PhysicsFilters, TVPreservesStepEdge) {
    Eigen::MatrixXf m = Eigen::MatrixXf::Zero(32, 32);
    m.rightCols(16).setConstant(400.0f);
    const auto out = PhysicsFilters::total_variation_denoise(m, 0.05f, 50);
    EXPECT_GT(std::abs(out(16,15) - out(16,16)), 300.0f);
}

TEST(PhysicsFilters, TVIdempotentOnDenoised) {
    Eigen::MatrixXf m = Eigen::MatrixXf::Constant(16, 16, 7.5f);
    const auto once = PhysicsFilters::total_variation_denoise(m, 0.1f, 30);
    const auto twice = PhysicsFilters::total_variation_denoise(once, 0.1f, 30);
    EXPECT_LT((once - twice).cwiseAbs().maxCoeff(), 1e-3f);
}

TEST(PhysicsFilters, PeronaMalikPreservesConstant) {
    Eigen::MatrixXf m = Eigen::MatrixXf::Constant(16, 16, 7.5f);
    const auto out = PhysicsFilters::perona_malik(m, 25.0f, 10);
    EXPECT_LT((out - m).cwiseAbs().maxCoeff(), 1e-4f);
}

TEST(PhysicsFilters, PeronaMalikReducesNoise) {
    const int N = 32;
    Eigen::MatrixXf clean = Eigen::MatrixXf::Constant(N, N, 50.0f);
    Eigen::MatrixXf noisy = clean;
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            noisy(i,j) += 10.0f * std::cos(0.9f*i * 1.1f*j);

    const float before = (noisy - clean).squaredNorm();
    const auto d = PhysicsFilters::perona_malik(noisy, 20.0f, 30);
    EXPECT_LT((d - clean).squaredNorm(), before);
}