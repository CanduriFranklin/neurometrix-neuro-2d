#pragma once
// BilateralSymmetry - Interhemispheric asymmetry detection.

#include <Eigen/Dense>
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace neurometrix::core2d::domain {

struct AsymmetryResult {
    double  asymmetry_score;   // 0..1, higher = more asymmetric
    int     peak_row;
    int     peak_col;
    double  peak_value;
};

class BilateralSymmetry {
public:
    // Split the image at the estimated midline, mirror, and compute
    // the pointwise difference. The largest difference region is the
    // most likely lesion.
    [[nodiscard]] static AsymmetryResult
    analyze(const Eigen::MatrixXf& img) {
        const int h = static_cast<int>(img.rows());
        const int w = static_cast<int>(img.cols());
        const int mid = w / 2;

        AsymmetryResult r{};
        r.asymmetry_score = 0.0;

        if (w < 4 || h < 4) return r;

        // Mirror left half onto right half and compute absolute difference.
        double total = 0.0;
        double peak = 0.0;
        int pr = 0, pc = 0;

        for (int i = 0; i < h; ++i) {
            for (int j = 0; j < mid; ++j) {
                const int jm = w - 1 - j;
                if (jm < 0) continue;
                const double diff = std::abs(
                    static_cast<double>(img(i, j)) -
                    static_cast<double>(img(i, jm)));
                total += diff;
                if (diff > peak) {
                    peak = diff;
                    pr = i;
                    pc = j;
                }
            }
        }

        const double denom = static_cast<double>(h) * static_cast<double>(mid);
        r.asymmetry_score = denom > 0 ? total / denom : 0.0;
        r.peak_row   = pr;
        r.peak_col   = pc;
        r.peak_value = peak;
        return r;
    }
};

} // namespace neurometrix::core2d::domain