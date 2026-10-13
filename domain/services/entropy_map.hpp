#pragma once
// EntropyMap - Local Shannon entropy for tissue heterogeneity.

#include <Eigen/Dense>
#include <cmath>
#include <cstdint>

namespace neurometrix::core2d::domain {

class EntropyMap {
public:
    // Local entropy in a window around each pixel.
    [[nodiscard]] static Eigen::MatrixXf
    compute(const Eigen::MatrixXf& img, int window = 9) {
        const int h = static_cast<int>(img.rows());
        const int w = static_cast<int>(img.cols());
        Eigen::MatrixXf out = Eigen::MatrixXf::Zero(h, w);
        const int r = window / 2;

        for (int i = 0; i < h; ++i) {
            for (int j = 0; j < w; ++j) {
                float lo = 1e30f, hi = -1e30f;
                for (int di = -r; di <= r; ++di)
                    for (int dj = -r; dj <= r; ++dj) {
                        const int ii = i + di, jj = j + dj;
                        if (ii < 0 || ii >= h || jj < 0 || jj >= w) continue;
                        const float v = img(ii, jj);
                        lo = std::min(lo, v);
                        hi = std::max(hi, v);
                    }
                const float range = std::max(1e-6f, hi - lo);

                constexpr int kBins = 32;
                int hist[kBins] = {0};
                int n = 0;
                for (int di = -r; di <= r; ++di)
                    for (int dj = -r; dj <= r; ++dj) {
                        const int ii = i + di, jj = j + dj;
                        if (ii < 0 || ii >= h || jj < 0 || jj >= w) continue;
                        const float norm = (img(ii, jj) - lo) / range;
                        int b = static_cast<int>(norm * (kBins - 1));
                        b = std::clamp(b, 0, kBins - 1);
                        hist[b]++;
                        n++;
                    }
                if (n == 0) continue;

                double H = 0.0;
                for (int b = 0; b < kBins; ++b) {
                    if (hist[b] == 0) continue;
                    const double p = static_cast<double>(hist[b]) / n;
                    H -= p * std::log2(p);
                }
                out(i, j) = static_cast<float>(H);
            }
        }
        return out;
    }
};

} // namespace neurometrix::core2d::domain