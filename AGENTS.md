# cascade

C++20 library for S-parameter measurements with traceable uncertainty. Status is
**pre-alpha scaffold**: only the version and value containers are implemented; no
calibration maths exists yet. Namespace is `cascade` throughout (renaming is
mechanical).

## Build & test

```sh
cmake --preset dev
cmake --build --preset dev
ctest --preset dev
```

Sanitizers (separate preset, inherits `dev`):

```sh
cmake --preset sanitize && cmake --build --preset sanitize && ctest --preset sanitize
```

Release: `cmake --preset release` then build/test with that preset.

Single test: `ctest --preset dev -R smoke` (or `--test-dir build/dev -R <name> --output-on-failure`).
Single target: `cmake --build --preset dev --target smoke_test` (also `header_check_<name>`).
CI (`.github/workflows/ci.yml`) builds gcc/clang x Debug/Release, a sanitizer job,
and a `clang-format --dry-run --Werror` job over `include src tests`.

## Requirements

C++20 compiler, CMake >= 3.24, Ninja, Eigen >= 3.4 (`find_package(Eigen3 3.4 REQUIRED NO_MODULE)`).

## Layout

- `include/cascade/` - public headers (interface only, no implementation)
- `src/` - implementation (only `version.cpp` today)
- `tests/` - plain executables plus generated standalone-header compile checks
- `fixtures/` - test data; `fixtures/vendor/` is gitignored (never commit vendor data)
- `docs/` - **empty in the committed tree**
- `local/` - gitignored; holds the real design/scope/verification docs and ADRs
  (`local/docs/...`). Do not assume they exist on a fresh clone.
- `reference/` - planned home of the Python oracle; not present yet

## Non-negotiable constraints (from CONTRIBUTING.md)

- Models take a scalar template parameter and **never return `double`**; never
  special-case a type in `include/cascade/`. Both break dual-number instantiation.
- No silent fallbacks: unknown standard model, file variant or distribution throws.
- A standard's definition is an **input quantity**, not a perturbation (else its
  Jacobian column is zero and the budget reports a confident `0.00 %`).
- No allocation in value types; no unordered iteration where it affects a result.
  Results must be bit-reproducible.
- Every public header must compile standalone with only its own includes (enforced
  by generated `header_check_*` tests in `tests/CMakeLists.txt`).
- `uncertainty.hpp` must not know about calibration types; `calibration.hpp` must
  not know about files.

## Architecture notes

- Layering (dependencies point down): measurements -> standards -> calibration model
  -> sensitivities -> covariance -> propagation -> I/O. See `local/docs/design.md`.
- Scalar polymorphism: every model is a template over a real scalar; instantiated
  with `double` for values and `Dual<T,N>` for sensitivities. Complex quantities
  are `Complex<T>` pairs, never `std::complex`.
- Covariance is blocked by influence and sparse over frequency; frequency is a batch
  dimension everywhere, never a loop around the model.
- Error handling uses exceptions (`cascade::Error` base planned); no error codes in
  arithmetic.
- Tests are dependency-free plain executables with return codes; no test framework
  is in use yet - adding one requires an ADR.

## Gotchas

- `local/`, `build/`, `build-*/` and `fixtures/vendor/` are gitignored.
- `docs/` is empty in the repo; the substantive docs live under `local/docs/`.
- `reference/` (Python oracle, ADR 0003) is not committed; parity tests that need
  it are skipped with an explanatory message.
- `clang-format` (checked-in `.clang-format`, 100-col limit) is enforced in CI;
  `clang-tidy` config (`.clang-tidy`) is checked in for local use.
- CMake presets set `CASCADE_WARNINGS_AS_ERRORS=ON` in `dev`; CI also passes it.
- Generated files live under `build/` (`header_*.cpp`, compile commands); do not
  edit them.

## References

`README.md`, `CONTRIBUTING.md`, `CMakePresets.json`, `fixtures/README.md`,
`.clang-format`, `.clang-tidy`, `local/docs/` (when present).
