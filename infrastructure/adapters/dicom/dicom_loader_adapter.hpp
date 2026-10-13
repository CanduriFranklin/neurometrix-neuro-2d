#pragma once

#include "../../../application/ports/outbound/i_dicom_loader.hpp"

#include <memory>

namespace neurometrix::core2d::infrastructure::dicom {

class DicomLoaderAdapter final : public application::IDicomLoader {
public:
    DicomLoaderAdapter() noexcept;
    ~DicomLoaderAdapter() override;

    DicomLoaderAdapter(const DicomLoaderAdapter&) = delete;
    DicomLoaderAdapter& operator=(const DicomLoaderAdapter&) = delete;

    [[nodiscard]] bool can_read(std::string_view path) const noexcept override;

    [[nodiscard]] std::expected<domain::Slice2D, application::DicomError>
    load(std::string_view path) const noexcept override;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace neurometrix::core2d::infrastructure::dicom