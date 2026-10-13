#pragma once
// PhysicsFilters - Edge-preserving denoising (TV + Perona-Malik).

#include <Eigen/Dense>
#include <algorithm>
#include <cmath>

namespace neurometrix::core2d::domain {

class PhysicsFilters {
public:
    [[nodiscard]] static Eigen::MatrixXf
    total_variation_denoise(const Eigen::MatrixXf& f,
                             float lambda = 0.1f, int iters = 50) {
        const int h = static_cast<int>(f.rows());
        const int w = static_cast<int>(f.cols());
        Eigen::MatrixXf u = f;
        Eigen::MatrixXf u_bar = f;
        Eigen::MatrixXf px = Eigen::MatrixXf::Zero(h, w);
        Eigen::MatrixXf py = Eigen::MatrixXf::Zero(h, w);
        const float tau = 0.25f, sigma = 0.25f;

        for (int it = 0; it < iters; ++it) {
            for (int i = 0; i < h; ++i) {
                for (int j = 0; j < w; ++j) {
                    const float gx = (j + 1 < w) ? (u_bar(i,j+1) - u_bar(i,j)) : 0.f;
                    const float gy = (i + 1 < h) ? (u_bar(i+1,j) - u_bar(i,j)) : 0.f;
                    float nx = px(i,j) + sigma * gx;
                    float ny = py(i,j) + sigma * gy;
                    const float nrm = std::max(1.0f,
                        std::sqrt(nx*nx + ny*ny) / std::max(lambda, 1e-6f));
                    px(i,j) = nx / nrm; py(i,j) = ny / nrm;
                }
            }
            Eigen::MatrixXf u_new = u;
            for (int i = 0; i < h; ++i)
                for (int j = 0; j < w; ++j) {
                    const float dx = px(i,j) - ((j>0)?px(i,j-1):0.f);
                    const float dy = py(i,j) - ((i>0)?py(i-1,j):0.f);
                    u_new(i,j) = u(i,j) + tau * (dx + dy);
                }
            u_bar = 2.f * u_new - u;
            u = u_new;
        }
        return u;
    }

    [[nodiscard]] static Eigen::MatrixXf
    perona_malik(const Eigen::MatrixXf& f, float kappa = 25.0f,
                  int iters = 20, float dt = 0.2f) {
        const int h = static_cast<int>(f.rows());
        const int w = static_cast<int>(f.cols());
        Eigen::MatrixXf u = f;
        auto g = [kappa](float s) { return 1.f / (1.f + (s*s)/(kappa*kappa)); };
        for (int it = 0; it < iters; ++it) {
            Eigen::MatrixXf u_new = u;
            for (int i = 1; i < h-1; ++i)
                for (int j = 1; j < w-1; ++j) {
                    const float dn = u(i-1,j) - u(i,j);
                    const float ds = u(i+1,j) - u(i,j);
                    const float de = u(i,j+1) - u(i,j);
                    const float dw = u(i,j-1) - u(i,j);
                    u_new(i,j) = u(i,j) + dt * (
                        g(std::abs(dn))*dn + g(std::abs(ds))*ds +
                        g(std::abs(de))*de + g(std::abs(dw))*dw);
                }
            u = u_new;
        }
        return u;
    }
};

} // namespace neurometrix::core2d::domain