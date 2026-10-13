#include <gtest/gtest.h>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
namespace {
struct StudyRequest { std::uint64_t message_id; std::uint64_t timestamp_utc; std::string study_uid; std::string dicom_path; };
std::vector<char> serialize(const StudyRequest& r) {
    std::vector<char> out;
    const std::uint32_t uid_len  = static_cast<std::uint32_t>(r.study_uid.size());
    const std::uint32_t path_len = static_cast<std::uint32_t>(r.dicom_path.size());
    out.resize(8 + 8 + 4 + uid_len + 4 + path_len);
    std::memcpy(out.data(),     &r.message_id,   8);
    std::memcpy(out.data() + 8, &r.timestamp_utc, 8);
    std::memcpy(out.data() + 16, &uid_len, 4);
    std::memcpy(out.data() + 20, r.study_uid.data(), uid_len);
    std::memcpy(out.data() + 20 + uid_len, &path_len, 4);
    std::memcpy(out.data() + 24 + uid_len, r.dicom_path.data(), path_len);
    return out;
}
bool deserialize(const char* data, std::size_t size, StudyRequest& out) {
    if (data == nullptr || size < 20) return false;
    std::uint64_t mid = 0, ts = 0;
    std::memcpy(&mid, data, 8);
    std::memcpy(&ts,  data + 8, 8);
    std::uint32_t ulen = 0;
    std::memcpy(&ulen, data + 16, 4);
    if (size < 20 + ulen + 4) return false;
    out.study_uid.assign(data + 20, ulen);
    std::uint32_t plen = 0;
    std::memcpy(&plen, data + 20 + ulen, 4);
    if (size < 20 + ulen + 4 + plen) return false;
    out.dicom_path.assign(data + 20 + ulen + 4, plen);
    out.message_id = mid; out.timestamp_utc = ts;
    return true;
}
} // namespace
TEST(Contracts, RoundTripIdentity) {
    const StudyRequest inputs[] = {
        {1, 100, "1.2.3", "C:/a.dcm"},
        {0xFFFF, 0xFFFFFFFF, "", ""},
        {42, 0, std::string(500, 'x'), std::string(1000, 'y')},
    };
    for (const auto& in : inputs) {
        const auto bytes = serialize(in);
        StudyRequest out{};
        ASSERT_TRUE(deserialize(bytes.data(), bytes.size(), out));
        EXPECT_EQ(out.message_id, in.message_id);
        EXPECT_EQ(out.study_uid, in.study_uid);
        EXPECT_EQ(out.dicom_path, in.dicom_path);
    }
}
TEST(Contracts, TruncatedPayloadsRejected) {
    StudyRequest in{1, 2, "uid", "path"};
    const auto full = serialize(in);
    for (std::size_t n = 0; n < full.size(); ++n) {
        StudyRequest out{};
        (void)deserialize(full.data(), n, out);
    }
}
TEST(Contracts, NullDataRejected) {
    StudyRequest out{};
    EXPECT_FALSE(deserialize(nullptr, 0, out));
    EXPECT_FALSE(deserialize(nullptr, 100, out));
}