#pragma once

#include "../../../contracts/messages.hpp"

#include <expected>
#include <cstdint>

namespace neurometrix::core2d::application {

enum class PublishError : std::uint8_t {
    BrokerUnavailable,
    Timeout,
    UnknownError,
};

class IMessagePublisher {
public:
    virtual ~IMessagePublisher() = default;

    virtual std::expected<void, PublishError>
    publish_result(const contracts::StudyResult& result) noexcept = 0;
};

} // namespace neurometrix::core2d::application