#pragma once

#include "../../../contracts/messages.hpp"

#include <expected>
#include <cstdint>

namespace neurometrix::core2d::application {

enum class UseCaseError : std::uint8_t {
    InputValidationFailed,
    DicomReadFailed,
    ComputeFailed,
    RepositoryUnavailable,
    PublisherUnavailable,
    InternalError,
};

class IProcessStudyUseCase {
public:
    virtual ~IProcessStudyUseCase() = default;

    virtual std::expected<contracts::StudyResult, UseCaseError>
    execute(const contracts::StudyRequest& request) noexcept = 0;
};

} // namespace neurometrix::core2d::application