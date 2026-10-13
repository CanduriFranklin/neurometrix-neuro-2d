#pragma once

#include "../../../infrastructure/adapters/config/config_loader.hpp"

#include <memory>

namespace neurometrix::core2d::infrastructure::metrics {

class MetricsServer {
public:
    explicit MetricsServer(const config::MetricsConfig& cfg) noexcept;
    ~MetricsServer();

    MetricsServer(const MetricsServer&) = delete;
    MetricsServer& operator=(const MetricsServer&) = delete;

    bool start() noexcept;
    void stop() noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace neurometrix::core2d::infrastructure::metrics