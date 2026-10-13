#pragma once
// HessianAnalysis - Frangi vesselness at multiple scales.

#include <Eigen/Dense>
#include <cmath>

namespace neurometrix::core2d::domain {

class HessianAnalysis {
public:
    [[nodiscard]] static Eigen::MatrixXf
    frangi_vesselness(const Eigen::MatrixXf& img,
                       float sigma_min = 1.0f,
                       float sigma_max = 4.0f,
                       int   num_sigmas = 4,
                       float beta = 0.5f,
                       float c = 0.5f) {
        const int h = static_cast<int>(img.rows());
        const int w = static_cast<int>(img.cols());
        Eigen::MatrixXf response = Eigen::MatrixXf::Zero(h, w);

        for (int s = 0; s < num_sigmas; ++s) {
            const float sigma = sigma_min +
                (sigma_max - sigma_min) * s / std::max(1, num_sigmas - 1);

            const Eigen::MatrixXf Dxx = second_derivative_xx(img, sigma);
            const Eigen::MatrixXf Dyy = second_derivative_yy(img, sigma);
            const Eigen::MatrixXf Dxy = second_derivative_xy(img, sigma);

            for (int i = 0; i < h; ++i) {
                for (int j = 0; j < w; ++j) {
                    const float dxx = Dxx(i,j);
                    const float dyy = Dyy(i,j);
                    const float dxy = Dxy(i,j);
                    const float tr = dxx + dyy;
                    const float det = dxx * dyy - dxy * dxy;
                    const float disc = std::sqrt(std::max(0.f, tr*tr/4.f - det));
                    float l1 = tr/2.f + disc;
                    float l2 = tr/2.f - disc;

                    if (std::abs(l1) > std::abs(l2)) std::swap(l1, l2);
                    if (l2 > 0.f) continue;  // bright on dark

                    const float Rb = std::abs(l1) / (std::abs(l2) + 1e-6f);
                    const float S  = std::sqrt(l1*l1 + l2*l2);
                    const float v  = std::exp(-(Rb*Rb)/(2.f*beta*beta))
                                   * (1.f - std::exp(-(S*S)/(2.f*c*c)));

                    if (v > response(i,j)) response(i,j) = v;
                }
            }
        }
        return response;
    }

private:
    [[nodiscard]] static Eigen::MatrixXf
    gaussian_blur(const Eigen::MatrixXf& img, float sigma) {
        const int h = static_cast<int>(img.rows());
        const int w = static_cast<int>(img.cols());
        const int r = std::max(1, static_cast<int>(std::ceil(3.0f * sigma)));
        Eigen::MatrixXf out(h, w);

        // 1D kernel
        std::vector<float> k(2*r + 1);
        float sum = 0.f;
        for (int i = -r; i <= r; ++i) {
            k[i+r] = std::exp(-(i*i)/(2.f*sigma*sigma));
            sum += k[i+r];
        }
        for (auto& v : k) v /= sum;

        // Horizontal
        for (int i = 0; i < h; ++i)
            for (int j = 0; j < w; ++j) {
                float acc = 0.f;
                for (int d = -r; d <= r; ++d) {
                    const int jj = std::clamp(j + d, 0, w - 1);
                    acc += img(i, jj) * k[d + r];
                }
                out(i, j) = acc;
            }
        // Vertical
        Eigen::MatrixXf out2(h, w);
        for (int i = 0; i < h; ++i)
            for (int j = 0; j < w; ++j) {
                float acc = 0.f;
                for (int d = -r; d <= r; ++d) {
                    const int ii = std::clamp(i + d, 0, h - 1);
                    acc += out(ii, j) * k[d + r];
                }
                out2(i, j) = acc;
            }
        return out2;
    }

    [[nodiscard]] static Eigen::MatrixXf
    second_derivative_xx(const Eigen::MatrixXf& img, float sigma) {
        const auto b = gaussian_blur(img, sigma);
        const int h = static_cast<int>(img.rows());
        const int w = static_cast<int>(img.cols());
        Eigen::MatrixXf d(h, w);
        for (int i = 0; i < h; ++i)
            for (int j = 1; j < w - 1; ++j)
                d(i, j) = b(i, j - 1) - 2.f*b(i, j) + b(i, j + 1);
        return d * sigma * sigma;
    }

    [[nodiscard]] static Eigen::MatrixXf
    second_derivative_yy(const Eigen::MatrixXf& img, float sigma) {
        const auto b = gaussian_blur(img, sigma);
        const int h = static_cast<int>(img.rows());
        const int w = static_cast<int>(img.cols());
        Eigen::MatrixXf d(h, w);
        for (int i = 1; i < h - 1; ++i)
            for (int j = 0; j < w; ++j)
                d(i, j) = b(i - 1, j) - 2.f*b(i, j) + b(i + 1, j);
        return d * sigma * sigma;
    }

    [[nodiscard]] static Eigen::MatrixXf
    second_derivative_xy(const Eigen::MatrixXf& img, float sigma) {
        const auto b = gaussian_blur(img, sigma);
        const int h = static_cast<int>(img.rows());
        const int w = static_cast<int>(img.cols());
        Eigen::MatrixXf d = Eigen::MatrixXf::Zero(h, w);
        for (int i = 1; i < h - 1; ++i)
            for (int j = 1; j < w - 1; ++j)
                d(i, j) = (b(i+1,j+1) - b(i+1,j-1) - b(i-1,j+1) + b(i-1,j-1)) * 0.25f;
        return d * sigma * sigma;
    }
};

} // namespace neurometrix::core2d::domain