#pragma once

// ============================================================================
// Slice2D - Value Object
// ============================================================================
// Represents a single 2D image slice loaded from a DICOM file.
// Uses Eigen for the pixel matrix.
// ============================================================================

#include <Eigen/Dense>
#include <cstdint>
#include <string>

namespace neurometrix::core2d::domain {

class Slice2D {
public:
    Slice2D(Eigen::MatrixXf pixels,
            std::uint32_t rows,
            std::uint32_t cols)
        : pixels_(std::move(pixels)),
          rows_(rows),
          cols_(cols) {}

    [[nodiscard]] const Eigen::MatrixXf& pixels() const noexcept { return pixels_; }
    [[nodiscard]] std::uint32_t rows() const noexcept { return rows_; }
    [[nodiscard]] std::uint32_t cols() const noexcept { return cols_; }
    [[nodiscard]] bool empty() const noexcept { return pixels_.size() == 0; }

private:
    Eigen::MatrixXf pixels_;
    std::uint32_t   rows_;
    std::uint32_t   cols_;
};

} // namespace neurometrix::core2d::domain