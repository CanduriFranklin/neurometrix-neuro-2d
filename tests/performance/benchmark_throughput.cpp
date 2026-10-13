#include "domain/services/physics_filters.hpp"
#include "domain/services/morphological_ops.hpp"
#include "domain/services/bilateral_symmetry.hpp"
#include "domain/services/entropy_map.hpp"
#include <chrono>
#include <cstdio>
#include <Eigen/Dense>
using namespace neurometrix::core2d::domain;
using namespace std::chrono;
int main() {
    constexpr int N = 512;
    constexpr int kBudgetSeconds = 5;
    Eigen::MatrixXf img(N, N);
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            img(i, j) = 40.0f + 20.0f * std::sin(i * 0.1f) * std::cos(j * 0.1f);
    const auto t_start = steady_clock::now();
    int processed = 0;
    while (duration_cast<seconds>(steady_clock::now() - t_start).count() < kBudgetSeconds) {
        const auto tv = PhysicsFilters::total_variation_denoise(img, 0.1f, 20);
        const auto th = MorphologicalOps::white_tophat(tv, 3);
        const auto sy = BilateralSymmetry::analyze(tv);
        const auto en = EntropyMap::compute(tv, 9);
        volatile float sink = th(0,0) + (float)sy.asymmetry_score + en(0,0);
        (void)sink;
        ++processed;
    }
    const auto elapsed = duration_cast<milliseconds>(steady_clock::now() - t_start).count();
    const double tput = 1000.0 * processed / static_cast<double>(elapsed);
    std::printf("Throughput: %.2f slices/s (%d in %lld ms)\n",
                tput, processed, static_cast<long long>(elapsed));
    return tput < 4.0 ? 1 : 0;
}