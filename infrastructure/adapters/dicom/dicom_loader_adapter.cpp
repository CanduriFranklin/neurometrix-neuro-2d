#include "dicom_loader_adapter.hpp"

#include "../../../infrastructure/adapters/logging/logger.hpp"

#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dctk.h>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace neurometrix::core2d::infrastructure::dicom {

namespace fs = std::filesystem;

namespace {

[[nodiscard]] std::string get_string(DcmDataset* ds, const DcmTagKey& tag) {
    OFString v;
    if (ds->findAndGetOFString(tag, v).good()) return std::string{v.c_str()};
    return {};
}

[[nodiscard]] double get_double(DcmDataset* ds, const DcmTagKey& tag, double fb) {
    Float64 v = 0.0;
    if (ds->findAndGetFloat64(tag, v).good()) return static_cast<double>(v);
    // Fallback: many tags are stored as IS/DS strings.
    OFString s;
    if (ds->findAndGetOFString(tag, s).good() && !s.empty()) {
        try { return std::stod(s.c_str()); } catch (...) {}
    }
    return fb;
}

[[nodiscard]] std::string to_upper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return s;
}

} // anonymous namespace

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

        // ---- Read clinical tags ----------------------------------------
        const std::string modality  = to_upper(get_string(ds, DCM_Modality));
        const std::string body_part = to_upper(get_string(ds, DCM_BodyPartExamined));
        const double slope     = get_double(ds, DCM_RescaleSlope, 1.0);
        const double intercept = get_double(ds, DCM_RescaleIntercept, 0.0);

        logging::get()->info("[dicom] modality='{}' body_part='{}' slope={} intercept={}",
                             modality, body_part, slope, intercept);

        // ---- Validate modality -----------------------------------------
        if (modality != "CT") {
            logging::get()->warn("[dicom] rejecting non-CT modality: '{}'", modality);
            return std::unexpected(application::DicomError::UnsupportedModality);
        }

        // ---- Validate body part (only if present) ----------------------
        if (!body_part.empty() &&
            body_part != "HEAD" && body_part != "BRAIN" &&
            body_part != "SKULL" && body_part != "NEURO") {
            logging::get()->warn("[dicom] rejecting non-head body part: '{}'", body_part);
            return std::unexpected(application::DicomError::UnsupportedBodyPart);
        }

        // ---- Read dimensions -------------------------------------------
        Uint16 rows = 0, cols = 0;
        if (ds->findAndGetUint16(DCM_Rows, rows).bad() ||
            ds->findAndGetUint16(DCM_Columns, cols).bad() ||
            rows == 0 || cols == 0) {
            return std::unexpected(application::DicomError::MissingMandatoryTag);
        }

        // ---- Read pixel data -------------------------------------------
        const Uint16* pixels = nullptr;
        unsigned long count = 0;
        const OFCondition ps = ds->findAndGetUint16Array(DCM_PixelData, pixels, &count);
        if (ps.bad() || pixels == nullptr || count == 0) {
            return std::unexpected(application::DicomError::CorruptedPixelData);
        }

        const std::size_t frame_pixels =
            static_cast<std::size_t>(rows) * static_cast<std::size_t>(cols);
        if (count < frame_pixels) {
            return std::unexpected(application::DicomError::CorruptedPixelData);
        }

        // ---- Convert RAW to Hounsfield Units ---------------------------
        // HU = raw * RescaleSlope + RescaleIntercept
        // This is the physical calibration that makes the pixel values
        // clinically meaningful (air ~ -1000, water ~ 0, bone ~ +900).
        Eigen::MatrixXf hu(rows, cols);
        for (std::uint32_t r = 0; r < rows; ++r) {
            for (std::uint32_t c = 0; c < cols; ++c) {
                const double raw = static_cast<double>(pixels[r * cols + c]);
                hu(r, c) = static_cast<float>(raw * slope + intercept);
            }
        }

        logging::get()->info("[dicom] loaded '{}': {}x{} HU[{}..{}]",
                             p.string(), rows, cols,
                             static_cast<double>(hu.minCoeff()),
                             static_cast<double>(hu.maxCoeff()));

        return domain::Slice2D(std::move(hu), rows, cols, modality, body_part);
    } catch (const std::bad_alloc&) {
        return std::unexpected(application::DicomError::CorruptedPixelData);
    } catch (...) {
        return std::unexpected(application::DicomError::UnknownError);
    }
}

} // namespace neurometrix::core2d::infrastructure::dicom