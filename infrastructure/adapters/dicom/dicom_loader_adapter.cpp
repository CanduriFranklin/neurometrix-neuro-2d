#include "dicom_loader_adapter.hpp"

#include "../../../infrastructure/adapters/logging/logger.hpp"

#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dctk.h>

#include <cstdint>
#include <cstring>
#include <filesystem>
#include <utility>
#include <vector>

namespace neurometrix::core2d::infrastructure::dicom {

namespace fs = std::filesystem;

struct DicomLoaderAdapter::Impl {};

DicomLoaderAdapter::DicomLoaderAdapter() noexcept
    : pimpl_(std::make_unique<Impl>()) {}

DicomLoaderAdapter::~DicomLoaderAdapter() = default;

bool DicomLoaderAdapter::can_read(std::string_view path) const noexcept {
    try {
        const fs::path p{std::string{path}};
        std::error_code ec;
        return fs::exists(p, ec) && !ec;
    } catch (...) {
        return false;
    }
}

std::expected<domain::Slice2D, application::DicomError>
DicomLoaderAdapter::load(std::string_view path) const noexcept {
    try {
        const fs::path p{std::string{path}};
        std::error_code ec;

        if (!fs::exists(p, ec) || ec) {
            return std::unexpected(application::DicomError::FileNotFound);
        }

        DcmFileFormat fmt;
        const OFCondition status = fmt.loadFile(p.string().c_str());
        if (!status.good()) {
            logging::get()->error("[dicom] loadFile failed: {}", status.text());
            return std::unexpected(application::DicomError::InvalidHeader);
        }

        DcmDataset* ds = fmt.getDataset();
        if (ds == nullptr) {
            return std::unexpected(application::DicomError::InvalidHeader);
        }

        Uint16 rows = 0, cols = 0;
        if (ds->findAndGetUint16(DCM_Rows, rows).bad() ||
            ds->findAndGetUint16(DCM_Columns, cols).bad() ||
            rows == 0 || cols == 0) {
            return std::unexpected(application::DicomError::MissingMandatoryTag);
        }

        const Uint16* pixels = nullptr;
        unsigned long count = 0;
        const OFCondition ps =
            ds->findAndGetUint16Array(DCM_PixelData, pixels, &count);
        if (ps.bad() || pixels == nullptr || count == 0) {
            return std::unexpected(application::DicomError::CorruptedPixelData);
        }

        // Take the first frame only (2D core).
        const std::size_t frame_pixels =
            static_cast<std::size_t>(rows) * static_cast<std::size_t>(cols);
        if (count < frame_pixels) {
            return std::unexpected(application::DicomError::CorruptedPixelData);
        }

        Eigen::MatrixXf mat(rows, cols);
        for (std::uint32_t r = 0; r < rows; ++r) {
            for (std::uint32_t c = 0; c < cols; ++c) {
                mat(r, c) = static_cast<float>(pixels[r * cols + c]);
            }
        }

        logging::get()->info("[dicom] loaded '{}': {}x{}", p.string(), rows, cols);
        return domain::Slice2D(std::move(mat), rows, cols);
    } catch (const std::bad_alloc&) {
        return std::unexpected(application::DicomError::CorruptedPixelData);
    } catch (...) {
        return std::unexpected(application::DicomError::UnknownError);
    }
}

} // namespace neurometrix::core2d::infrastructure::dicom