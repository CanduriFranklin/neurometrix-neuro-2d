#pragma once

#include "../../../application/ports/outbound/i_message_publisher.hpp"
#include "../../../infrastructure/adapters/config/config_loader.hpp"
#include "../../../infrastructure/adapters/security/tls_context.hpp"

#include <memory>

namespace neurometrix::core2d::infrastructure::kafka {

class KafkaProducerAdapter final : public application::IMessagePublisher {
public:
    KafkaProducerAdapter(
        const config::KafkaConfig&  config,
        const security::TLSContext& tls
    ) noexcept;

    ~KafkaProducerAdapter() override;

    KafkaProducerAdapter(const KafkaProducerAdapter&) = delete;
    KafkaProducerAdapter& operator=(const KafkaProducerAdapter&) = delete;

    bool start() noexcept;
    void stop() noexcept;

    [[nodiscard]] std::expected<void, application::PublishError>
    publish_result(const contracts::StudyResult& result) noexcept override;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace neurometrix::core2d::infrastructure::kafka