#include "domain/services/hessian_analysis.hpp"
#include <gtest/gtest.h>
#include <Eigen/Dense>
using namespace neurometrix::core2d::domain;
TEST(Hessian, VesselnessBounded) {
    Eigen::MatrixXf m = Eigen::MatrixXf::Random(32, 32);
    const auto v = HessianAnalysis::frangi_vesselness(m);
    EXPECT_GE(v.minCoeff(), 0.0f);
    EXPECT_LE(v.maxCoeff(), 1.0f);
}
TEST(Hessian, ConstantIsSilent) {
    Eigen::MatrixXf m = Eigen::MatrixXf::Constant(32, 32, 100.0f);
    const auto v = HessianAnalysis::frangi_vesselness(m);
    EXPECT_LT(v.maxCoeff(), 0.05f);
}
TEST(Hessian, DetectsElongatedStructure) {
    Eigen::MatrixXf m = Eigen::MatrixXf::Zero(64, 64);
    for (int i = 0; i < 64; ++i)
        for (int j = 30; j < 33; ++j) m(i, j) = 100.0f;
    const auto v = HessianAnalysis::frangi_vesselness(m, 1.0f, 4.0f, 4);
    EXPECT_GT(v.col(31).maxCoeff(), 0.05f);
}