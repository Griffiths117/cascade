// cascade — S-parameter containers
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <array>
#include <cstddef>
#include <utility>
#include <vector>

#include "cascade/scalar.hpp"

namespace cascade {

/// Fixed-size S-parameter matrix, row-major, over an arbitrary real scalar.
///
/// Entries are `Complex<T>`, so `SMatrix<double, 2>` is an ordinary two-port and
/// `SMatrix<Dual<...>, 2>` is the same two-port carrying sensitivities. The
/// template parameter is the *real* scalar, never the complex type: that is what
/// lets the entire model be instantiated twice from one source.
///
/// Row index is the receiving port, column index the source port, matching the
/// `S[i][j]` convention used in file formats: `S(1,0)` is S21.
///
/// Fixed size on purpose: port counts are known at compile time for the
/// calibrations this library supports (1 and 2 for v0.1), and an `std::array`
/// keeps the whole model free of allocation and friendly to the dual-number
/// instantiation.
template<Scalar T, int Ports>
struct SMatrix {
    static_assert(Ports >= 1, "at least one port");
    static constexpr int kPorts = Ports;
    static constexpr std::size_t kEntries =
        static_cast<std::size_t>(Ports) * static_cast<std::size_t>(Ports);

    using value_type = Complex<T>;
    std::array<value_type, kEntries> data{};

    [[nodiscard]] constexpr value_type& operator()(int i, int j) {
        return data[static_cast<std::size_t>(i) * static_cast<std::size_t>(Ports) +
                    static_cast<std::size_t>(j)];
    }
    [[nodiscard]] constexpr const value_type& operator()(int i, int j) const {
        return data[static_cast<std::size_t>(i) * static_cast<std::size_t>(Ports) +
                    static_cast<std::size_t>(j)];
    }
    [[nodiscard]] constexpr bool operator==(const SMatrix&) const = default;
};

template<int Ports>
using SMatrixD = SMatrix<double, Ports>;

using S1 = SMatrixD<1>;
using S2 = SMatrixD<2>;

/// A frequency grid in hertz, assumed non-decreasing and non-empty.
///
/// Models are evaluated over a whole grid at once rather than point by point.
/// The reason is not speed: influences are correlated *between* frequencies, so a
/// per-point API would make the interesting uncertainty questions unanswerable.
/// `Grid::size()` is the batch dimension of every model.
class Grid {
public:
    Grid() = default;
    explicit Grid(std::vector<double> frequencies_hz) : f_(std::move(frequencies_hz)) {}

    [[nodiscard]] std::size_t size() const noexcept { return f_.size(); }
    [[nodiscard]] bool empty() const noexcept { return f_.empty(); }
    [[nodiscard]] double operator[](std::size_t i) const { return f_[i]; }
    [[nodiscard]] const std::vector<double>& frequencies() const noexcept { return f_; }

private:
    std::vector<double> f_;
};

/// Port numbering, following the convention in the VNA Tools data formats: a port
/// has an index, a mode (single-ended, differential, common) and a within-mode
/// index. Only single-ended is supported in v0.1, but the field exists so that
/// mixed-mode support does not change the data model.
struct Port {
    int index{0};       ///< 1-based port number as printed on the instrument
    int mode{0};        ///< 0 = single-ended, 1 = differential, 2 = common
    int mode_index{0};  ///< 0-based index within the mode
};

/// Reference impedance of a measurement plane, in ohms, carried rather than
/// assumed: renormalisation needs it, and it is occasionally not 50.
struct ReferenceImpedance {
    double real_ohm{50.0};
    double imag_ohm{0.0};
};

}  // namespace cascade
