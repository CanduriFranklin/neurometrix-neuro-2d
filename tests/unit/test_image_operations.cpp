#include "domain/services/image_operations.hpp"

#include <gtest/gtest.h>

#include <Eigen/Dense>

using neurometrix::core2d::domain::ImageOperations;

TEST(ImageOperations, MetricsOnConstantImage) {
    Eigen::MatrixXf m = Eigen::MatrixXf::Constant(8, 8, 5.0f);
    const auto metrics = ImageOperations::compute_metrics(m);
    EXPECT_NEAR(metrics.mean,   5.0, 1e-6);
    EXPECT_NEAR(metrics.stddev, 0.0, 1e-6);
    EXPECT_NEAR(metrics.min,    5.0, 1e-6);
    EXPECT_NEAR(metrics.max,    5.0, 1e-6);
}

TEST(ImageOperations, GaussianBlurPreservesConstantImage) {
    Eigen::MatrixXf m = Eigen::MatrixXf::Constant(8, 8, 3.0f);
    const auto blurred = ImageOperations::gaussian_blur_3x3(m);
    EXPECT_NEAR(blurred.mean(), 3.0f, 1e-5);
}