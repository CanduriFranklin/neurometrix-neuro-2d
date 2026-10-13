#include "domain/services/physics_filters.hpp"
#include "domain/services/morphological_ops.hpp"
#include "domain/services/entropy_map.hpp"
#include <cstdio>
#include <Eigen/Dense>
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif
using namespace neurometrix::core2d::domain;
static double rss_mb() {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
        return pmc.WorkingSetSize / (1024.0 * 1024.0);
#endif
    return 0.0;
}
int main() {
    const double before = rss_mb();
    std::printf("RSS before: %.2f MB\n", before);
    constexpr int N = 512;
    for (int it = 0; it < 50; ++it) {
        Eigen::MatrixXf img = Eigen::MatrixXf::Constant(N, N, 40.0f);
        const auto tv = PhysicsFilters::total_variation_denoise(img, 0.1f, 10);
        const auto th = MorphologicalOps::white_tophat(tv, 3);
        const auto en = EntropyMap::compute(tv, 9);
        volatile float sink = th(0,0) + en(0,0);
        (void)sink;
    }
    const double after = rss_mb();
    std::printf("RSS after:  %.2f MB\n", after);
    std::printf("Delta:      %.2f MB\n", after - before);
    return (after - before > 50.0) ? 1 : 0;
}