#pragma once

// ============================================================================
// ImageOperations - Pure domain service
// ============================================================================
// Stateless 2D image mathematics on Eigen matrices.
// Header-only for now. When ROCm is enabled, replace the .eval() calls
// with GPU-dispatched kernels without changing the public API.
// ============================================================================

#include <Eigen/Dense>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

namespace neurometrix::core2d::domain {

struct ImageMetrics {
    double mean{0.0};
    double stddev{0.0};
    double min{0.0};
    double max{0.0};
    double entropy{0.0};
};

class ImageOperations {
public:
    // 3x3 Gaussian blur (separable, applied row then column).
    [[nodiscard]] static Eigen::MatrixXf
    gaussian_blur_3x3(const Eigen::MatrixXf& img) {
        const Eigen::Matrix<float, 3, 1> k =
            (Eigen::Matrix<float, 3, 1>() << 0.25f, 0.5f, 0.25f).finished();

        Eigen::MatrixXf rows = img;
        for (int r = 0; r < rows.rows(); ++r) {
            for (int c = 1; c < rows.cols() - 1; ++c) {
                rows(r, c) = 0.25f * img(r, c - 1)
                           + 0.5f  * img(r, c)
                           + 0.25f * img(r, c + 1);
            }
        }

        Eigen::MatrixXf out = rows;
        for (int r = 1; r < rows.rows() - 1; ++r) {
            for (int c = 0; c < rows.cols(); ++c) {
                out(r, c) = 0.25f * rows(r - 1, c)
                          + 0.5f  * rows(r, c)
                          + 0.25f * rows(r + 1, c);
            }
        }
        return out;
    }

    // Intensity metrics over the whole matrix.
    [[nodiscard]] static ImageMetrics
    compute_metrics(const Eigen::MatrixXf& img) {
        ImageMetrics m{};
        if (img.size() == 0) return m;

        m.mean = static_cast<double>(img.mean());
        m.min  = static_cast<double>(img.minCoeff());
        m.max  = static_cast<double>(img.maxCoeff());

        double sq_sum = 0.0;
        for (int i = 0; i < img.size(); ++i) {
            const double d = static_cast<double>(img(i)) - m.mean;
            sq_sum += d * d;
        }
        m.stddev = std::sqrt(sq_sum / static_cast<double>(img.size()));

        // Shannon entropy on a 256-bin histogram.
        constexpr int kBins = 256;
        std::array<std::size_t, kBins> hist{};
        const double range = std::max(1e-9, m.max - m.min);
        for (int i = 0; i < img.size(); ++i) {
            const double norm = (static_cast<double>(img(i)) - m.min) / range;
            int bin = static_cast<int>(norm * (kBins - 1));
            bin = std::clamp(bin, 0, kBins - 1);
            hist[bin]++;
        }
        const double n = static_cast<double>(img.size());
        double entropy = 0.0;
        for (auto h : hist) {
            if (h == 0) continue;
            const double p = static_cast<double>(h) / n;
            entropy -= p * std::log2(p);
        }
        m.entropy = entropy;
        return m;
    }
};

} // namespace neurometrix::core2d::domain