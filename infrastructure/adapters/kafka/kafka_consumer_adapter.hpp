#pragma once

#include "../../../application/ports/inbound/i_process_study_use_case.hpp"
#include "../../../infrastructure/adapters/config/config_loader.hpp"
#include "../../../infrastructure/adapters/security/tls_context.hpp"

#include <atomic>
#include <memory>

namespace neurometrix::core2d::infrastructure::kafka {

// ============================================================================
// KafkaConsumerAdapter
// ============================================================================
// Consumes binary StudyRequest messages from KAFKA_TOPIC_IN, deserializes
// them, and dispatches to the use case. Runs a blocking poll loop.
// ============================================================================
class KafkaConsumerAdapter {
public:
    KafkaConsumerAdapter(
        const config::KafkaConfig&         config,
        const security::TLSContext&        tls,
        application::IProcessStudyUseCase& use_case
    ) noexcept;

    ~KafkaConsumerAdapter();

    KafkaConsumerAdapter(const KafkaConsumerAdapter&) = delete;
    KafkaConsumerAdapter& operator=(const KafkaConsumerAdapter&) = delete;

    bool run() noexcept;
    void stop() noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace neurometrix::core2d::infrastructure::kafka