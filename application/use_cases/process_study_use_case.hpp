#pragma once

#include "../../contracts/messages.hpp"
#include "../ports/inbound/i_process_study_use_case.hpp"
#include "../ports/outbound/i_dicom_loader.hpp"
#include "../ports/outbound/i_job_repository.hpp"
#include "../ports/outbound/i_message_publisher.hpp"

#include <cstdint>

namespace neurometrix::core2d::application {

// ============================================================================
// ProcessStudyUseCase
// ============================================================================
// Orchestrates the 2D processing of a single study:
//   1. Validate input
//   2. Mark job as processing in Postgres
//   3. Load DICOM slice from disk
//   4. Apply 2D image operations (Eigen, optional ROCm)
//   5. Persist metrics to Postgres
//   6. Publish result to Kafka
//   7. Return StudyResult
// ============================================================================
class ProcessStudyUseCase final : public IProcessStudyUseCase {
public:
    ProcessStudyUseCase(
        const IDicomLoader&     dicom_loader,
        IJobRepository&         repository,
        IMessagePublisher&      publisher
    ) noexcept;

    [[nodiscard]] std::expected<contracts::StudyResult, UseCaseError>
    execute(const contracts::StudyRequest& request) noexcept override;

private:
    [[nodiscard]] std::expected<void, UseCaseError>
    validate(const contracts::StudyRequest& request) const noexcept;

    const IDicomLoader& dicom_loader_;
    IJobRepository&     repository_;
    IMessagePublisher&  publisher_;
};

} // namespace neurometrix::core2d::application