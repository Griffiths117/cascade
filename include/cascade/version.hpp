// cascade — version information
// SPDX-License-Identifier: Apache-2.0
#pragma once

namespace cascade {

/// Library version as a string, e.g. "0.0.1".
[[nodiscard]] const char* version_string() noexcept;

/// Major/minor/patch, kept in step with CMakeLists.txt by the build.
struct Version {
    int major;
    int minor;
    int patch;
};
[[nodiscard]] Version version() noexcept;

}  // namespace cascade
