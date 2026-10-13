#include "domain/entities/finding.hpp"
#include <gtest/gtest.h>
#include <cstring>
using namespace neurometrix::core2d::domain;
TEST(Finding, ToStringCoversAllValues) {
    for (auto t : {FindingType::MicroHemorrhage, FindingType::AcuteHemorrhage,
                   FindingType::IschemicChange, FindingType::MidlineShift,
                   FindingType::VentricularDilation, FindingType::MassEffect,
                   FindingType::Calcification, FindingType::Unknown}) {
        const char* s = to_string(t);
        ASSERT_NE(s, nullptr);
        EXPECT_GT(std::strlen(s), 0u);
    }
}
TEST(Finding, DefaultValuesAreSafe) {
    Finding f{};
    EXPECT_EQ(f.type, FindingType::Unknown);
    EXPECT_DOUBLE_EQ(f.confidence, 0.0);
    EXPECT_TRUE(f.note.empty());
}