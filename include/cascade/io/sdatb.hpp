// cascade — METAS .sdatb reading and writing
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <string>

#include "cascade/io/dataset.hpp"
#include "cascade/io/format.hpp"

namespace cascade::io {

/// Read one .sdatb file (v1: values only; v2: values + covariance).
[[nodiscard]] SData read_sdatb(const std::string& path);

/// Sniff the sdatb format version (1-5) from a file without fully parsing it.
[[nodiscard]] int sdatb_format_version(const std::string& path);

/// Write one .sdatb file. v2 when covariance is present, v1 otherwise.
void write_sdatb(const std::string& path, const SData& data);

}  // namespace cascade::io
