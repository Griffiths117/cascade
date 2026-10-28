# Style guide

How to write code in `cascade`. Everything here is extracted from the existing
tree; where a rule has a checker, the checker is named. When this guide and the
code disagree, fix the code or fix this guide — do not leave them divergent.

The domain-specific rules (scalar polymorphism, no silent fallbacks, standards
as input quantities, bit-reproducibility) live in `CONTRIBUTING.md` and are not
repeated in full here. This document is about *how to write*; that one is about
*what must be true*.

## Formatting

`clang-format` with the checked-in `.clang-format` is authoritative; run it and
do not hand-argue with it. The notable settings, so you know what to expect:

- LLVM base, C++20, 4-space indent, no tabs, 100-column limit.
- Braces attach (`namespace cascade {` opens on the same line).
- Pointer alignment left: `const char* version_string()`.
- Template declarations always break onto their own line:

  ```cpp
  template<class T, std::size_t N>
  [[nodiscard]] constexpr Dual<T, N> operator*(const Dual<T, N>& a, const Dual<T, N>& b);
  ```

- Short single-line function bodies are allowed **only** for `inline`-style
  definitions; short `if` statements are never collapsed to one line.
- Include blocks are preserved; includes are sorted case-sensitively within a
  block.
- Two spaces before trailing comments.

CI runs `clang-format --dry-run --Werror` over `include src tests`. Format
before committing; the check is exact, not advisory.

Editor basics (`.editorconfig`): UTF-8, LF, final newline, no trailing
whitespace. Source files are ASCII — write `1 +/- 5 %` in a comment, not
`1 ± 5 %` (see `uncertainty.hpp`).

## File layout and headers

Every source file starts with the two-line header, then `#pragma once` for
headers (see any file in `include/cascade/`):

```cpp
// cascade — <short description, lowercase>
// SPDX-License-Identifier: Apache-2.0
#pragma once
```

- Public headers are interface-only: declarations plus templates. Non-template
  implementations go in `src/`.
- A header includes what it uses — nothing more, nothing that a downstream
  file is expected to provide. Every public header must compile standalone;
  this is enforced by the generated `header_check_*` tests, so a new public
  header must be added to `tests/CMakeLists.txt` (`CASCADE_PUBLIC_HEADERS`, or
  the io list below it). A header that fails this test is not committed.
- Include order, as in the existing files:
  - In a **header**: standard library first, blank line, then project headers
    (`cascade/...`, quoted).
  - In a **.cpp**: the module's own headers first (unquoted project form), so
    self-containment failures surface immediately; blank line; then the
    standard library.
- `include/cascade/` holds no implementation; `src/` holds no public
  declarations. Never `#include` a `.cpp`.

## Naming

Enforced by `.clang-tidy` (`readability-identifier-naming`); matches the tree:

| Thing              | Case         | Examples                                  |
|--------------------|--------------|-------------------------------------------|
| Types (class/struct/enum) | CamelCase | `Dual`, `LinearisationFlag`, `Budget` |
| Functions, methods | `lower_case` | `read_sdatb`, `total_variance`            |
| Variables, members | `lower_case` | `value`, `grad`                           |
| Private members     | `_` suffix   | `influences_`                             |
| Enum values         | `lower_case` | `Kind::normal`, `FileFormat::sdatb`       |
| Concepts            | CamelCase    | `Scalar`                                  |
| Namespaces          | `lower_case` | `cascade`, `cascade::io`                  |

No Hungarian notation, no `I`-prefixes, no `m_` prefix. A function-local alias
may shorten a long standard name (`namespace fs = std::filesystem;` inside the
function, as in `src/io.cpp`).

## C++ usage

- C++20. Use concepts to constrain template parameters
  (`template<Scalar T> struct Complex`), not `static_assert` prose.
- `constexpr` wherever a definition can be; value types are usable at compile
  time.
- `[[nodiscard]]` on every function whose return value must not be dropped —
  effectively all const accessors and all pure computations.
- `noexcept` only when genuinely true, and say so sparingly.
- Single-argument constructors are `explicit`, except when the conversion is the
  point (`Complex(T real, T imag)` is two-argument anyway; `Dual(T v)` is
  `explicit`).
- Defaulted special members preferred: `constexpr Complex() = default;`,
  `operator==(const Complex&) const = default;`.
- Fixed-width integer types (`std::uint32_t`, not `unsigned long`);
  `std::size_t` for sizes and indexing.
- No magic integers (`cppcoreguidelines-avoid-magic-numbers` is on, floating
  point literals are exempt — tolerances may be written as `1e-15` in place).
- File-local helpers in a `.cpp` go in an anonymous namespace near the top, and
  the namespace closes with a comment: `}  // namespace cascade`.
- No C arrays, no manual new/delete. Storage is either a value-type member, a
  `std::array` (fixed size, as in `Dual::grad`), or a `std::vector` for
  variable-size data owned by non-value types.

### Domain rules that shape the code

These have checkers only partially, so they are on you:

- **Scalar polymorphism.** Every model is a template over a real scalar,
  instantiated with `double` and `Dual<T, N>`. It never returns `double`, never
  special-cases a concrete scalar. `Complex<T>` pairs are used instead of
  `std::complex`, because a dual is not an arithmetic type.
- **Value types do not allocate.** Types copied into duals keep their storage
  fixed; heap storage appears only in owning containers outside the model.
- **No unordered iteration where it affects a result.** Map iteration order
  must never leak into an output; results are bit-reproducible.
- **Frequency is a batch dimension**, never a loop around the model.
- **Layering** (dependencies point down): measurements → standards →
  calibration model → sensitivities → covariance → propagation → I/O. The
  enforced special cases: `uncertainty.hpp` must not know about calibration
  types; `calibration.hpp` must not know about files.

## Error handling

Exceptions, not error codes, and no silent fallbacks anywhere:

- Unknown standard, unknown file variant, unknown distribution, malformed
  input: throw, with a message that says what and names the file where
  relevant: `throw UnknownFormat("cannot open file: " + path);`.
- A `switch` over an enum handles every enumerator explicitly and carries no
  `default:` label, so `-Wswitch` keeps working; anything left over falls
  through to a trailing throw (see `read_dataset` in `src/io.cpp`).
- Error types derive from the planned `cascade::Error` base; the I/O layer
  already has `UnknownFormat`, `MalformedFile`, `UnsupportedFeature`. Add a
  specific type rather than reusing a vague one.
- Returning a guessed value where an error is possible is the one
  unforgivable failure mode — it produces a plausible wrong number.

## Documentation comments

`///` for entities, `///<` trailing for members. The house rule is **why, not
what**: the code already says what. The good examples are in the tree:

- `scalar.hpp` opens with the central design decision and what would break it.
- `uncertainty.hpp` cites prototype measurements in doc comments ("a factor of
  7.6", "the ratio fall to 0.76 and coverage to 93 %"). Numbers with provenance
  beat adjectives.
- Planned-but-unimplemented shapes are marked `TODO(v0.1):` with the intended
  design, not a bare `TODO`.

A doc comment that only paraphrases the signature should be deleted. A comment
inside a function explains the non-obvious step (see the chain-rule comments in
`Dual::operator*=`).

## Tests

- Framework: Catch2 v3 (ADR 0004). Unit tests live in `tests/unit/*.cpp` and
  are globbed into `cascade_tests` — a new file is picked up automatically,
  no `CMakeLists.txt` edit needed.
- `TEST_CASE` titles are sentences describing behaviour ("Dual matches central
  differences"), with tags: `[unit][scalar]`.
- Exact values use `REQUIRE` (`REQUIRE(sum.value == 5.0)`); floating-point
  comparisons use `REQUIRE_THAT(... , Catch::Matchers::WithinAbs(expected, tol))`
  with a tolerance chosen for the magnitude, and a comment on the non-obvious
  expectations.
- A new public header gets its `header_check_*` entry (see above).
- Order of tests for new maths: round-trip (recover a known device exactly)
  before uncertainty; an implementation that cannot round-trip has nothing to
  say about its Jacobian.
- Parity tests that need the Python oracle under `reference/` are skipped with
  an explanatory message when it is absent — never deleted.
- Every result-affecting change reports the measured number, not "tests pass"
  (CONTRIBUTING rule 4).

## Tooling and workflow

```sh
cmake --preset dev && cmake --build --preset dev && ctest --preset dev
cmake --preset sanitize && cmake --build --preset sanitize && ctest --preset sanitize
```

- `dev` builds with warnings as errors; treat a warning as a failure locally
  too.
- `clang-tidy` is advisory locally, enforced narrow and configured in
  `.clang-tidy`; fix what it reports in headers it filters
  (`HeaderFilterRegex: include/cascade/.*`).
- Never edit anything under `build/` — generated `header_*.cpp` and compile
  commands live there.
- Generated files, vendor data (`fixtures/vendor/`), and everything under
  `local/` are never committed.

## Commits

- Small, self-contained, committed as soon as verified (tests + format clean),
  not accumulated.
- Imperative subject lines that state the why; reference an ADR or issue where
  one exists.
- Decisions that need a derivation (e.g. branch selection in an eigenvalue
  decomposition) get an ADR *before* the code — that trigger is in
  `CONTRIBUTING.md`.
