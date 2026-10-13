#include "domain/services/physics_filters.hpp"
#include "domain/services/morphological_ops.hpp"
#include "domain/services/hessian_analysis.hpp"
#include "domain/services/bilateral_symmetry.hpp"
#include "domain/services/entropy_map.hpp"
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <Eigen/Dense>
#include <vector>
#include <omp.h>

using namespace neurometrix::core2d::domain;
using namespace std::chrono;

static Eigen::MatrixXf make_slice(int n) {
    Eigen::MatrixXf m(n, n);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j) {
            const float base = 40.0f;
            const float s = 20.0f * std::sin(i * 0.1f) * std::cos(j * 0.1f);
            const float nz = 5.0f * std::fmod(std::sin(i*12.9898f + j*78.233f) * 43758.5f, 1.0f);
            m(i, j) = base + s + nz;
        }
    return m;
}

static double pct(const std::vector<double>& v, double p) {
    if (v.empty()) return 0.0;
    const std::size_t i = static_cast<std::size_t>((v.size() - 1) * p / 100.0);
    return v[i];
}

int main() {
    std::printf("[diag] OpenMP max threads = %d\n", omp_get_max_threads());
    constexpr int kIter = 20;
    const int sizes[] = {256, 512, 1024};
    int failed = 0;

    for (int size : sizes) {
        const auto img = make_slice(size);
        std::vector<double> times;
        times.reserve(kIter);

        for (int it = 0; it < kIter; ++it) {
            const auto t0 = steady_clock::now();
            const auto tv = PhysicsFilters::total_variation_denoise(img, 0.1f, 10);
            const auto th = MorphologicalOps::white_tophat(tv, 3);
            const auto vs = HessianAnalysis::frangi_vesselness(tv, 1.0f, 4.0f, 3);
            const auto sy = BilateralSymmetry::analyze(tv);
            const auto en = EntropyMap::compute(tv, 9);
            volatile float sink = th(0,0) + vs(0,0) + (float)sy.asymmetry_score + en(0,0);
            (void)sink;
            const auto t1 = steady_clock::now();
            times.push_back(duration_cast<microseconds>(t1 - t0).count() / 1000.0);
        }
        std::sort(times.begin(), times.end());

        std::printf("=== %dx%d ===\n", size, size);
        std::printf("  p50 = %8.2f ms\n", pct(times, 50));
        std::printf("  p95 = %8.2f ms\n", pct(times, 95));
        std::printf("  p99 = %8.2f ms\n", pct(times, 99));
        std::printf("  max = %8.2f ms\n", times.back());

        const double sla = (size == 512) ? 5000.0 : (size == 256 ? 1000.0 : 20000.0);
        const bool ok = pct(times, 99) <= sla;
        std::printf("  SLA = %8.2f ms  -> %s\n", sla, ok ? "PASS" : "FAIL");
        if (!ok) failed++;
    }
    return failed;
}