#pragma once
// Slice2D - Value Object representing a single CT slice in HU.

#include <Eigen/Dense>
#include <cstdint>
#include <string>
#include <utility>

namespace neurometrix::core2d::domain {

class Slice2D {
public:
    Slice2D(Eigen::MatrixXf hu_pixels,
            std::uint32_t rows,
            std::uint32_t cols,
            std::string modality,
            std::string body_part)
        : hu_(std::move(hu_pixels)),
          rows_(rows),
          cols_(cols),
          modality_(std::move(modality)),
          body_part_(std::move(body_part)) {}

    // Pixel values already in Hounsfield Units (physical scale).
    [[nodiscard]] const Eigen::MatrixXf& hu() const noexcept { return hu_; }

    // Backward-compatibility alias for existing call sites.
    [[nodiscard]] const Eigen::MatrixXf& pixels() const noexcept { return hu_; }

    [[nodiscard]] std::uint32_t rows() const noexcept { return rows_; }
    [[nodiscard]] std::uint32_t cols() const noexcept { return cols_; }
    [[nodiscard]] bool empty() const noexcept { return hu_.size() == 0; }

    [[nodiscard]] const std::string& modality() const noexcept { return modality_; }
    [[nodiscard]] const std::string& body_part() const noexcept { return body_part_; }

private:
    Eigen::MatrixXf hu_;
    std::uint32_t   rows_;
    std::uint32_t   cols_;
    std::string     modality_;
    std::string     body_part_;
};

} // namespace neurometrix::core2d::domain