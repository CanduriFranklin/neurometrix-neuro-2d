#pragma once
#include <prometheus/counter.h>
#include <prometheus/gauge.h>
#include <prometheus/histogram.h>
#include <prometheus/registry.h>
#include <memory>
#include <string>
namespace neurometrix::core2d::infrastructure::metrics {
class MetricsRegistry {
public:
    explicit MetricsRegistry(std::shared_ptr<prometheus::Registry> reg);
    void inc_messages_consumed() noexcept;
    void inc_messages_processed() noexcept;
    void inc_messages_failed() noexcept;
    void observe_pipeline_ms(double ms) noexcept;
    void observe_dicom_load_ms(double ms) noexcept;
    void observe_kafka_publish_ms(double ms) noexcept;
    void observe_asymmetry_score(double s) noexcept;
    void observe_mean_hu(double hu) noexcept;
    void inc_finding(const std::string& type) noexcept;
    void set_gpu_used(bool used) noexcept;
private:
    std::shared_ptr<prometheus::Registry> registry_;
    prometheus::Counter* consumed_{nullptr};
    prometheus::Counter* processed_{nullptr};
    prometheus::Counter* failed_{nullptr};
    prometheus::Histogram* pipeline_hist_{nullptr};
    prometheus::Histogram* dicom_hist_{nullptr};
    prometheus::Histogram* kafka_hist_{nullptr};
    prometheus::Histogram* asymmetry_hist_{nullptr};
    prometheus::Histogram* hu_hist_{nullptr};
    prometheus::Family<prometheus::Counter>* finding_family_{nullptr};
    prometheus::Gauge* gpu_used_{nullptr};
};
} // namespace neurometrix::core2d::infrastructure::metrics