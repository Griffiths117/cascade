// cascade — smoke test
// SPDX-License-Identifier: Apache-2.0
//
// Purpose: prove the toolchain, the include path and the value types work, and
// pin the one thing that is implemented (the version). It is not a maths test —
// the maths does not exist yet.

#include <cmath>
#include <cstdio>
#include <cstring>

#include "cascade/cascade.hpp"

namespace {

int failures = 0;

void check(bool condition, const char* what) {
    if (!condition) {
        std::printf("FAIL  %s\n", what);
        ++failures;
    } else {
        std::printf("ok    %s\n", what);
    }
}

}  // namespace

int main() {
    using namespace cascade;

    check(std::strcmp(version_string(), "0.0.1") == 0,
          "version string matches CMake project version");

    const Version v = version();
    check(v.major == 0 && v.minor == 0 && v.patch == 1, "version struct matches");

    // The S-parameter container addresses entries in the file-format convention.
    S2 s;
    s(1, 0) = ComplexDouble{0.5, -0.25};  // S21
    s(0, 1) = ComplexDouble{0.5, 0.25};   // S12
    check(s(1, 0).re == 0.5 && s(0, 1).im == 0.25, "SMatrix indexing is S[row][column]");
    check(S2::kEntries == 4, "two-port matrix holds four entries");

    // Complex arithmetic, checked against a hand computation:
    // (1 + 2j)(3 - 1j) = 5 + 5j
    const ComplexDouble a{1.0, 2.0};
    const ComplexDouble b{3.0, -1.0};
    const ComplexDouble prod = a * b;
    check(std::abs(prod.re - 5.0) < 1e-15 && std::abs(prod.im - 5.0) < 1e-15, "complex multiply");

    // A grid is a batch; an empty grid is allowed to exist but reports empty.
    const Grid g{std::vector<double>{1e9, 2e9, 3e9}};
    check(g.size() == 3 && !g.empty(), "grid size");

    if (failures != 0) {
        std::printf("%d check(s) failed\n", failures);
        return 1;
    }
    std::printf("all checks passed\n");
    return 0;
}
