#pragma once
// Finding - Clinical finding entity.

#include <cstdint>
#include <string>

namespace neurometrix::core2d::domain {

enum class FindingType : std::uint8_t {
    MicroHemorrhage,
    AcuteHemorrhage,
    IschemicChange,
    MidlineShift,
    VentricularDilation,
    MassEffect,
    Calcification,
    Unknown,
};

[[nodiscard]] inline const char* to_string(FindingType t) noexcept {
    switch (t) {
        case FindingType::MicroHemorrhage:     return "micro_hemorrhage";
        case FindingType::AcuteHemorrhage:     return "acute_hemorrhage";
        case FindingType::IschemicChange:      return "ischemic_change";
        case FindingType::MidlineShift:        return "midline_shift";
        case FindingType::VentricularDilation: return "ventricular_dilation";
        case FindingType::MassEffect:          return "mass_effect";
        case FindingType::Calcification:       return "calcification";
        case FindingType::Unknown:             return "unknown";
    }
    return "unknown";
}

struct Finding {
    FindingType  type{FindingType::Unknown};
    double       confidence{0.0};
    int          row{0};
    int          col{0};
    double       hu_value{0.0};
    double       area_pixels{0.0};
    std::string  note;
};

} // namespace neurometrix::core2d::domain