// cascade — version information
// SPDX-License-Identifier: Apache-2.0
#include "cascade/version.hpp"

#ifndef CASCADE_VERSION_MAJOR
#define CASCADE_VERSION_MAJOR 0
#endif
#ifndef CASCADE_VERSION_MINOR
#define CASCADE_VERSION_MINOR 0
#endif
#ifndef CASCADE_VERSION_PATCH
#define CASCADE_VERSION_PATCH 0
#endif

#define CASCADE_DETAIL_STRINGIFY_(x) #x
#define CASCADE_DETAIL_STRINGIFY(x) CASCADE_DETAIL_STRINGIFY_(x)

namespace cascade {
namespace {
constexpr char kVersionString[] =
    CASCADE_DETAIL_STRINGIFY(CASCADE_VERSION_MAJOR) "." CASCADE_DETAIL_STRINGIFY(
        CASCADE_VERSION_MINOR) "." CASCADE_DETAIL_STRINGIFY(CASCADE_VERSION_PATCH);
}  // namespace

const char* version_string() noexcept {
    return kVersionString;
}

Version version() noexcept {
    return {CASCADE_VERSION_MAJOR, CASCADE_VERSION_MINOR, CASCADE_VERSION_PATCH};
}

}  // namespace cascade
