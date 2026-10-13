#pragma once

#include "../../../domain/value_objects/slice_2d.hpp"

#include <cstdint>
#include <expected>
#include <string_view>

namespace neurometrix::core2d::application {

enum class DicomError : std::uint8_t {
    FileNotFound,
    InvalidHeader,
    MissingMandatoryTag,
    CorruptedPixelData,
    UnsupportedModality,        // not CT
    UnsupportedBodyPart,        // not HEAD/BRAIN
    IOError,
    UnknownError,
};

class IDicomLoader {
public:
    virtual ~IDicomLoader() = default;

    virtual bool can_read(std::string_view path) const noexcept = 0;

    virtual std::expected<domain::Slice2D, DicomError>
    load(std::string_view path) const noexcept = 0;
};

} // namespace neurometrix::core2d::application