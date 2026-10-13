#pragma once
// MorphologicalOps - Top-hat transform for micro-hemorrhage detection.

#include <Eigen/Dense>
#include <algorithm>
#include <vector>

namespace neurometrix::core2d::domain {

class MorphologicalOps {
public:
    [[nodiscard]] static Eigen::MatrixXf
    dilate(const Eigen::MatrixXf& f, int radius = 1) {
        const int h = static_cast<int>(f.rows());
        const int w = static_cast<int>(f.cols());
        Eigen::MatrixXf out(h, w);
        for (int i = 0; i < h; ++i)
            for (int j = 0; j < w; ++j) {
                float m = f(i,j);
                for (int di = -radius; di <= radius; ++di)
                    for (int dj = -radius; dj <= radius; ++dj) {
                        const int ii = i + di, jj = j + dj;
                        if (ii >= 0 && ii < h && jj >= 0 && jj < w)
                            m = std::max(m, f(ii, jj));
                    }
                out(i,j) = m;
            }
        return out;
    }

    [[nodiscard]] static Eigen::MatrixXf
    erode(const Eigen::MatrixXf& f, int radius = 1) {
        const int h = static_cast<int>(f.rows());
        const int w = static_cast<int>(f.cols());
        Eigen::MatrixXf out(h, w);
        for (int i = 0; i < h; ++i)
            for (int j = 0; j < w; ++j) {
                float m = f(i,j);
                for (int di = -radius; di <= radius; ++di)
                    for (int dj = -radius; dj <= radius; ++dj) {
                        const int ii = i + di, jj = j + dj;
                        if (ii >= 0 && ii < h && jj >= 0 && jj < w)
                            m = std::min(m, f(ii, jj));
                    }
                out(i,j) = m;
            }
        return out;
    }

    [[nodiscard]] static Eigen::MatrixXf
    opening(const Eigen::MatrixXf& f, int r = 1) {
        return dilate(erode(f, r), r);
    }

    [[nodiscard]] static Eigen::MatrixXf
    closing(const Eigen::MatrixXf& f, int r = 1) {
        return erode(dilate(f, r), r);
    }

    // White top-hat: features SMALLER than the structuring element.
    // Perfect for microhemorrhages (bright spots 1-5 mm).
    [[nodiscard]] static Eigen::MatrixXf
    white_tophat(const Eigen::MatrixXf& f, int r = 3) {
        return f - opening(f, r);
    }

    // Black top-hat: dark features smaller than the element.
    // Useful for detecting hypodense ischemic regions.
    [[nodiscard]] static Eigen::MatrixXf
    black_tophat(const Eigen::MatrixXf& f, int r = 3) {
        return closing(f, r) - f;
    }
};

} // namespace neurometrix::core2d::domain