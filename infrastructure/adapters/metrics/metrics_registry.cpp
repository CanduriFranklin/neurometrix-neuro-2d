#include "metrics_registry.hpp"
namespace neurometrix::core2d::infrastructure::metrics {
MetricsRegistry::MetricsRegistry(std::shared_ptr<prometheus::Registry> reg)
    : registry_(std::move(reg)) {
    consumed_ = &prometheus::BuildCounter()
        .Name("core2d_messages_consumed_total")
        .Help("Messages consumed from Kafka").Register(*registry_).Add({});
    processed_ = &prometheus::BuildCounter()
        .Name("core2d_messages_processed_total")
        .Help("Messages fully processed").Register(*registry_).Add({});
    failed_ = &prometheus::BuildCounter()
        .Name("core2d_messages_failed_total")
        .Help("Messages that failed").Register(*registry_).Add({});
    pipeline_hist_ = &prometheus::BuildHistogram()
        .Name("core2d_pipeline_duration_ms")
        .Help("End-to-end pipeline latency")
        .Register(*registry_).Add({}, prometheus::Histogram::BucketBoundaries{
            10, 25, 50, 100, 200, 500, 1000, 2000});
    dicom_hist_ = &prometheus::BuildHistogram()
        .Name("core2d_dicom_load_duration_ms")
        .Help("DICOM load latency").Register(*registry_)
        .Add({}, prometheus::Histogram::BucketBoundaries{1, 5, 10, 25, 50, 100, 250});
    kafka_hist_ = &prometheus::BuildHistogram()
        .Name("core2d_kafka_publish_duration_ms")
        .Help("Kafka publish latency").Register(*registry_)
        .Add({}, prometheus::Histogram::BucketBoundaries{1, 5, 10, 25, 50, 100, 250});
    asymmetry_hist_ = &prometheus::BuildHistogram()
        .Name("core2d_asymmetry_score")
        .Help("Interhemispheric asymmetry").Register(*registry_)
        .Add({}, prometheus::Histogram::BucketBoundaries{0.01, 0.05, 0.1, 0.2, 0.5, 1.0, 2.0});
    hu_hist_ = &prometheus::BuildHistogram()
        .Name("core2d_mean_hu")
        .Help("Mean HU").Register(*registry_)
        .Add({}, prometheus::Histogram::BucketBoundaries{-100, 0, 10, 20, 40, 60, 80, 100, 200});
    finding_family_ = &prometheus::BuildCounter()
        .Name("core2d_findings_total")
        .Help("Clinical findings by type").Register(*registry_);
    gpu_used_ = &prometheus::BuildGauge()
        .Name("core2d_gpu_used")
        .Help("1 if GPU used").Register(*registry_).Add({});
}
void MetricsRegistry::inc_messages_consumed() noexcept { consumed_->Increment(); }
void MetricsRegistry::inc_messages_processed() noexcept { processed_->Increment(); }
void MetricsRegistry::inc_messages_failed() noexcept { failed_->Increment(); }
void MetricsRegistry::observe_pipeline_ms(double ms) noexcept { pipeline_hist_->Observe(ms); }
void MetricsRegistry::observe_dicom_load_ms(double ms) noexcept { dicom_hist_->Observe(ms); }
void MetricsRegistry::observe_kafka_publish_ms(double ms) noexcept { kafka_hist_->Observe(ms); }
void MetricsRegistry::observe_asymmetry_score(double s) noexcept { asymmetry_hist_->Observe(s); }
void MetricsRegistry::observe_mean_hu(double hu) noexcept { hu_hist_->Observe(hu); }
void MetricsRegistry::inc_finding(const std::string& type) noexcept {
    finding_family_->Add({{"type", type}}).Increment();
}
void MetricsRegistry::set_gpu_used(bool used) noexcept {
    gpu_used_->Set(used ? 1.0 : 0.0);
}
} // namespace neurometrix::core2d::infrastructure::metrics