// cascade — file format detection and dispatch
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <stdexcept>
#include <string>

#include "cascade/io/dataset.hpp"

namespace cascade::io {

enum class FileFormat {
    auto_detect,
    sdatb,
    touchstone_v1,
    touchstone_v2,
    citi,
};

struct FormatCapabilities {
    bool reads_values{true};
    bool reads_uncertainty{false};
    bool reads_port_impedance{false};
    bool reads_frequency_conversion{false};
    bool writes_values{true};
    bool writes_uncertainty{false};
    bool writes_port_impedance{false};
    bool writes_frequency_conversion{false};
};

[[nodiscard]] constexpr FormatCapabilities capabilities(FileFormat format) noexcept {
    switch (format) {
    case FileFormat::sdatb:
        return {true, true, true, true, true, true, true, true};
    case FileFormat::touchstone_v1:
    case FileFormat::touchstone_v2:
        return {true, false, false, false, true, false, false, false};
    case FileFormat::citi:
        return {true, false, false, false, false, false, false, false};
    case FileFormat::auto_detect:
        return {false, false, false, false, false, false, false, false};
    }
    return {};
}

/// Thrown when a file's format cannot be determined.
struct UnknownFormat : std::runtime_error {
    using std::runtime_error::runtime_error;
};

/// Thrown when a file is truncated or malformed.
struct MalformedFile : std::runtime_error {
    using std::runtime_error::runtime_error;
};

/// Thrown when an operation needs a feature the format does not support
/// (e.g. writing covariance to Touchstone) without an explicit opt-in.
struct UnsupportedFeature : std::runtime_error {
    using std::runtime_error::runtime_error;
};

/// Thrown when a consumer requires uncertainty data but the dataset has none.
struct MissingUncertainty : std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct MissingPortImpedance : std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct MissingFrequencyConversion : std::runtime_error {
    using std::runtime_error::runtime_error;
};

/// Detect a file's format from extension and/or content. Throws `UnknownFormat`
/// when nothing matches.
[[nodiscard]] FileFormat detect_format(const std::string& path);

/// Read any supported file into the common `SData` representation.
///
/// `format == auto_detect` sniffs the file; any other value forces that reader.
/// Throws `UnknownFormat`, `MalformedFile`, or `UnsupportedFeature` as appropriate.
[[nodiscard]] SData read_dataset(const std::string& path,
                                 FileFormat format = FileFormat::auto_detect);

}  // namespace cascade::io
