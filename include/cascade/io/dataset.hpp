// cascade — dataset: the common in-memory representation for loaded S-parameter data
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <optional>
#include <string>
#include <vector>

#include "cascade/matrix.hpp"
#include "cascade/uncertainty.hpp"

namespace cascade::io {

/// Frequency conversion subset (METAS sdatb v4/v5).
struct FrequencyConversionSub {
    double numerator{1.0};
    double denominator{1.0};
    double offset{0.0};
};

/// Full frequency conversion (METAS sdatb v5): one per port.
struct FrequencyConversion {
    FrequencyConversionSub test_receiver;
    FrequencyConversionSub reference_receiver;
    FrequencyConversionSub source;
};

/// Common in-memory representation produced by every reader.
///
/// `covariance` is `std::nullopt` when the source format carries no dependency
/// information (e.g. Touchstone). That absence is explicit: readers never
/// substitute a zero covariance, and consumers that need a budget must check.
struct SData {
    Grid grid;
    std::vector<Port> ports;
    /// Per-port reference (port) impedance. Empty when the format has none.
    std::vector<ReferenceImpedance> port_impedances;
    std::vector<S2> values;                ///< one entry per frequency
    std::optional<Covariance> covariance;  ///< nullopt = no uncertainty in source
    /// Frequency conversion subsets (sdatb v4): one per port. Empty when absent.
    std::optional<std::vector<FrequencyConversionSub>> frequency_conversion_subsets;
    /// Full frequency conversions (sdatb v5): one per port. Empty when absent.
    std::optional<std::vector<FrequencyConversion>> frequency_conversions;
    std::string format_id;    ///< e.g. "sdatb", "touchstone-1.0"
    std::string source_path;  ///< empty for in-memory data
    int format_version{0};
};

}  // namespace cascade::io
