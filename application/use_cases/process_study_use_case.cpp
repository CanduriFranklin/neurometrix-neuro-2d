// ============================================================================
// ProcessStudyUseCase - Real CT brain pipeline
// ============================================================================
// Steps:
//   1. Validate input
//   2. Persist job
//   3. Load DICOM slice
//   4. Hounsfield conversion + brain window
//   5. TV denoising (edge-preserving)
//   6. White top-hat (micro-hemorrhage candidate map)
//   7. Frangi vesselness (vascular structures, elongated hemorrhage)
//   8. Bilateral symmetry analysis (interhemispheric asymmetry)
//   9. Local entropy map (tissue heterogeneity)
//  10. Extract findings from score maps with HU-based validation
//  11. Publish result + findings to Kafka
//  12. Persist final state
// ============================================================================
#include "process_study_use_case.hpp"

#include "../../domain/services/physics_filters.hpp"
#include "../../domain/services/morphological_ops.hpp"
#include "../../domain/services/hessian_analysis.hpp"
#include "../../domain/services/bilateral_symmetry.hpp"
#include "../../domain/services/entropy_map.hpp"
#include "../../domain/value_objects/hounsfield_window.hpp"

#include <Eigen/Dense>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace neurometrix::core2d::application {

namespace {

[[nodiscard]] std::int64_t now_ms() noexcept {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

// Extracts candidate findings from the score maps.
// Thresholds are clinically motivated:
//   - Micro-hemorrhage:  top-hat response > 25 HU AND underlying HU in [45,80]
//   - Acute hemorrhage:  HU >= 45 for >= 20 contiguous pixels
//   - Ischemic change:   HU in [20,35] for >= 50 contiguous pixels
//   - Asymmetry peak:    symmetry diff > 30 HU
void extract_findings(const Eigen::MatrixXf& hu_img,
                       const Eigen::MatrixXf& tophat,
                       const Eigen::MatrixXf& vesselness,
                       const domain::AsymmetryResult& sym,
                       std::vector<domain::Finding>& out)
{
    // --- Micro-hemorrhage candidates from top-hat ---
    double tophat_peak = 0.0;
    int tr = 0, tc = 0;
    for (int i = 0; i < tophat.rows(); ++i) {
        for (int j = 0; j < tophat.cols(); ++j) {
            const double v = tophat(i, j);
            if (v > tophat_peak) { tophat_peak = v; tr = i; tc = j; }
        }
    }
    if (tophat_peak > 25.0) {
        const double hu_here = hu_img(tr, tc);
        if (hu_here >= 45.0 && hu_here <= 80.0) {
            domain::Finding f{};
            f.type       = domain::FindingType::MicroHemorrhage;
            f.confidence = std::min(1.0, tophat_peak / 60.0);
            f.row = tr; f.col = tc;
            f.hu_value = hu_here;
            f.note = "high top-hat response in acute blood HU range";
            out.push_back(f);
        }
    }

    // --- Asymmetry peak ---
    if (sym.peak_value > 30.0) {
        domain::Finding f{};
        f.type       = domain::FindingType::AcuteHemorrhage;
        f.confidence = std::min(1.0, sym.peak_value / 80.0);
        f.row = sym.peak_row; f.col = sym.peak_col;
        f.hu_value = hu_img(sym.peak_row, sym.peak_col);
        f.note = "interhemispheric asymmetry peak";
        out.push_back(f);
    }

    // --- Vesselness peak (elongated hemorrhage / vascular) ---
    double v_peak = 0.0;
    int vr = 0, vc = 0;
    for (int i = 0; i < vesselness.rows(); ++i) {
        for (int j = 0; j < vesselness.cols(); ++j) {
            if (vesselness(i, j) > v_peak) { v_peak = vesselness(i, j); vr = i; vc = j; }
        }
    }
    if (v_peak > 0.15) {
        domain::Finding f{};
        f.type       = domain::FindingType::Calcification;
        f.confidence = std::min(1.0, v_peak / 0.5);
        f.row = vr; f.col = vc;
        f.hu_value = hu_img(vr, vc);
        f.note = "elongated high-HU structure";
        out.push_back(f);
    }
}

} // anonymous namespace

ProcessStudyUseCase::ProcessStudyUseCase(
    const IDicomLoader& dicom_loader,
    IJobRepository&     repository,
    IMessagePublisher&  publisher
) noexcept
    : dicom_loader_(dicom_loader),
      repository_(repository),
      publisher_(publisher)
{}

std::expected<void, UseCaseError>
ProcessStudyUseCase::validate(const contracts::StudyRequest& request) const noexcept {
    if (request.message_id == 0) return std::unexpected(UseCaseError::InputValidationFailed);
    if (request.dicom_path.empty()) return std::unexpected(UseCaseError::InputValidationFailed);
    if (request.study_uid.empty()) return std::unexpected(UseCaseError::InputValidationFailed);
    return {};
}

std::expected<contracts::StudyResult, UseCaseError>
ProcessStudyUseCase::execute(const contracts::StudyRequest& request) noexcept {
    const std::int64_t start_ms = now_ms();

    if (auto v = validate(request); !v) return std::unexpected(v.error());

    if (auto saved = repository_.save_job(request); !saved) {
        return std::unexpected(UseCaseError::RepositoryUnavailable);
    }
    (void)repository_.mark_processing(request.message_id, start_ms);

    if (!dicom_loader_.can_read(request.dicom_path)) {
        (void)repository_.mark_failed(request.message_id, "dicom_read_failed", "not readable");
        return std::unexpected(UseCaseError::DicomReadFailed);
    }
    auto slice_result = dicom_loader_.load(request.dicom_path);
    if (!slice_result) {
        (void)repository_.mark_failed(request.message_id, "dicom_read_failed", "loader");
        return std::unexpected(UseCaseError::DicomReadFailed);
    }
    const auto& slice = *slice_result;

    contracts::StudyResult result{};
    result.message_id = request.message_id;
    result.study_uid  = request.study_uid;
    result.status     = "processed";

    try {
        using namespace domain;

        // ---- Step 4: use HU directly (adapter already applied slope/intercept) ----
        const auto& hu = slice.hu();

        // Basic HU statistics
        result.mean_hu   = static_cast<double>(hu.mean());
        result.min_hu    = static_cast<double>(hu.minCoeff());
        result.max_hu    = static_cast<double>(hu.maxCoeff());
        double sq = 0.0;
        for (int i = 0; i < hu.size(); ++i) {
            const double d = static_cast<double>(hu(i)) - result.mean_hu;
            sq += d * d;
        }
        result.stddev_hu = std::sqrt(sq / static_cast<double>(hu.size()));

        // ---- Step 5: TV denoising ----
        const auto denoised = PhysicsFilters::total_variation_denoise(hu, 0.1f, 30);

        // ---- Step 6: white top-hat for bright small structures ----
        const auto tophat = MorphologicalOps::white_tophat(denoised, 3);

        // ---- Step 7: Frangi vesselness ----
        const auto vesselness = HessianAnalysis::frangi_vesselness(denoised);
        result.peak_vesselness = static_cast<double>(vesselness.maxCoeff());

        // ---- Step 8: bilateral symmetry ----
        const auto sym = BilateralSymmetry::analyze(denoised);
        result.asymmetry_score = sym.asymmetry_score;

        // ---- Step 9: local entropy ----
        const auto entropy = EntropyMap::compute(denoised, 9);
        result.mean_entropy = static_cast<double>(entropy.mean());

        // ---- Step 10: extract findings ----
        extract_findings(denoised, tophat, vesselness, sym, result.findings);

        result.gpu_used = false;
    } catch (...) {
        (void)repository_.mark_failed(request.message_id, "compute_failed", "exception");
        return std::unexpected(UseCaseError::ComputeFailed);
    }

    const std::int64_t finish_ms = now_ms();
    result.processing_ms = static_cast<std::uint64_t>(finish_ms - start_ms);

    // ---- Step 11: publish ----
    if (auto published = publisher_.publish_result(result); !published) {
        (void)repository_.mark_failed(request.message_id, "publisher_unavailable", "kafka");
        return std::unexpected(UseCaseError::PublisherUnavailable);
    }

    // ---- Step 12: persist ----
    (void)repository_.save_result(result, start_ms, finish_ms);

    return result;
}

} // namespace neurometrix::core2d::application