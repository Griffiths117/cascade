// cascade — Touchstone (.sNp) reading
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <string>

#include "cascade/io/dataset.hpp"

namespace cascade::io {

/// Read a Touchstone 1.0/2.0 file. Values only: covariance is `std::nullopt`.
///
/// TODO: implement. Unknown or malformed files must throw.
[[nodiscard]] SData read_touchstone(const std::string& path);

}  // namespace cascade::io
