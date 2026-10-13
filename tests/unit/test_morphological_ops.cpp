#include "domain/services/morphological_ops.hpp"
#include <gtest/gtest.h>
#include <Eigen/Dense>

using namespace neurometrix::core2d::domain;

TEST(Morphology, DilationIncreasesOrEquals) {
    Eigen::MatrixXf m = Eigen::MatrixXf::Random(16, 16);
    const auto d = MorphologicalOps::dilate(m, 2);
    EXPECT_TRUE((d.array() >= m.array() - 1e-6f).all());
}

TEST(Morphology, ErosionDecreasesOrEquals) {
    Eigen::MatrixXf m = Eigen::MatrixXf::Random(16, 16);
    const auto e = MorphologicalOps::erode(m, 2);
    EXPECT_TRUE((e.array() <= m.array() + 1e-6f).all());
}

TEST(Morphology, OpeningIsAntiExtensive) {
    Eigen::MatrixXf m = Eigen::MatrixXf::Random(16, 16);
    const auto o = MorphologicalOps::opening(m, 2);
    EXPECT_TRUE((o.array() <= m.array() + 1e-6f).all());
}

TEST(Morphology, ClosingIsExtensive) {
    Eigen::MatrixXf m = Eigen::MatrixXf::Random(16, 16);
    const auto c = MorphologicalOps::closing(m, 2);
    EXPECT_TRUE((c.array() >= m.array() - 1e-6f).all());
}

TEST(Morphology, WhiteTopHatConstantIsZero) {
    Eigen::MatrixXf m = Eigen::MatrixXf::Constant(32, 32, 5.0f);
    const auto th = MorphologicalOps::white_tophat(m, 3);
    EXPECT_LT(th.cwiseAbs().maxCoeff(), 1e-4f);
}

TEST(Morphology, WhiteTopHatDetectsBrightSpot) {
    Eigen::MatrixXf m = Eigen::MatrixXf::Zero(32, 32);
    for (int i = 15; i <= 17; ++i)
        for (int j = 15; j <= 17; ++j)
            m(i, j) = 100.0f;
    const auto th = MorphologicalOps::white_tophat(m, 3);
    EXPECT_GT(th(16, 16), 50.0f);
}

TEST(Morphology, BlackTopHatDetectsDarkSpot) {
    Eigen::MatrixXf m = Eigen::MatrixXf::Constant(32, 32, 100.0f);
    for (int i = 15; i <= 17; ++i)
        for (int j = 15; j <= 17; ++j)
            m(i, j) = 0.0f;
    const auto th = MorphologicalOps::black_tophat(m, 3);
    EXPECT_GT(th(16, 16), 50.0f);
}