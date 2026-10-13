#include "domain/value_objects/hounsfield_window.hpp"
#include <gtest/gtest.h>
#include <cmath>

using namespace neurometrix::core2d::domain;

// INVARIANT: HU conversion is affine.
TEST(Hounsfield, AffineConversion) {
    for (double slope : {1.0, 0.5, 2.0}) {
        for (double intercept : {-1024.0, -100.0, 0.0, 100.0}) {
            const auto hu = HounsfieldWindow::to_hu(1024, slope, intercept);
            EXPECT_DOUBLE_EQ(hu, 1024.0 * slope + intercept);
        }
    }
}

// INVARIANT: window mapping is monotone non-decreasing.
TEST(Hounsfield, WindowMappingMonotone) {
    const auto w = window_of(WindowPreset::Brain);
    double prev = -1e9;
    for (double hu = -200; hu <= 200; hu += 1.0) {
        const double v = HounsfieldWindow::apply_window(hu, w);
        EXPECT_GE(v, prev - 1e-9) << "monotonicity broken at HU=" << hu;
        EXPECT_GE(v, 0.0);
        EXPECT_LE(v, 1.0);
        prev = v;
    }
}

// INVARIANT: window mapping is exactly 0.5 at WL.
TEST(Hounsfield, WindowCenterIsHalf) {
    for (auto preset : {WindowPreset::Brain, WindowPreset::Blood,
                        WindowPreset::Subdural, WindowPreset::Stroke}) {
        const auto w = window_of(preset);
        EXPECT_NEAR(HounsfieldWindow::apply_window(w.level, w), 0.5, 1e-9);
    }
}

// INVARIANT: all clinical window presets are valid (WW > 0).
TEST(Hounsfield, PresetsHavePositiveWidth) {
    for (auto p : {WindowPreset::Brain, WindowPreset::Blood, WindowPreset::Bone,
                   WindowPreset::Stroke, WindowPreset::Subdural}) {
        const auto w = window_of(p);
        EXPECT_GT(w.width, 0.0);
    }
}

// INVARIANT: classification is total (every HU has a label).
TEST(Hounsfield, ClassificationTotal) {
    for (double hu = -2000; hu <= 3000; hu += 25.0) {
        EXPECT_FALSE(HounsfieldWindow::classify_hu(hu).empty());
    }
}

// INVARIANT: acute blood and ischemia never overlap.
TEST(Hounsfield, ClinicalRangesDisjoint) {
    for (double hu = -500; hu <= 500; hu += 0.1) {
        EXPECT_FALSE(HounsfieldWindow::is_acute_blood(hu) &&
                     HounsfieldWindow::is_ischemic_risk(hu));
    }
}