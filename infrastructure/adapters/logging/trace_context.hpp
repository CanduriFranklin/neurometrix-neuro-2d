#pragma once
#include <atomic>
#include <cstdint>
#include <string>
namespace neurometrix::core2d::infrastructure::logging {
class TraceContext {
public:
    static void set(std::uint64_t message_id, const std::string& study_uid) noexcept {
        message_id_.store(message_id, std::memory_order_relaxed);
        uid_() = study_uid;
    }
    static std::uint64_t message_id() noexcept {
        return message_id_.load(std::memory_order_relaxed);
    }
    static const std::string& study_uid() noexcept { return uid_(); }
    static void clear() noexcept {
        message_id_.store(0, std::memory_order_relaxed);
        uid_().clear();
    }
private:
    static std::string& uid_() noexcept {
        static thread_local std::string s;
        return s;
    }
    inline static std::atomic<std::uint64_t> message_id_{0};
};
} // namespace neurometrix::core2d::infrastructure::logging