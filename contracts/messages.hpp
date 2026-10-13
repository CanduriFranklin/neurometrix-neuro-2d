#pragma once
// Wire contracts between the Router and the Core 2D.
// Input: binary. Output: JSON.

#include "../domain/entities/finding.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace neurometrix::core2d::contracts {

struct StudyRequest {
    std::uint64_t message_id{0};
    std::uint64_t timestamp_utc{0};
    std::string   study_uid;
    std::string   dicom_path;
};

struct StudyResult {
    std::uint64_t message_id{0};
    std::string   study_uid;
    std::string   status;

    // Basic metrics in Hounsfield Units.
    double mean_hu{0.0};
    double stddev_hu{0.0};
    double min_hu{0.0};
    double max_hu{0.0};

    // Advanced metrics.
    double asymmetry_score{0.0};
    double mean_entropy{0.0};
    double peak_vesselness{0.0};

    // Clinical findings extracted from the pipeline.
    std::vector<domain::Finding> findings;

    std::uint64_t processing_ms{0};
    bool          gpu_used{false};
    std::string   error_detail;
};

} // namespace neurometrix::core2d::contracts