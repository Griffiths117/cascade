# cascade

S-parameter measurements with traceable uncertainty. A library for computing what
a vector network analyzer measurement *is*, and how well it is known.

Status: **pre-alpha, scaffold only.** The public interface is sketched, nothing is
implemented. The working prototype that this library will be ported from lives in
a separate spike (`vnaunc-spike`), which stays in service as the reference
implementation and test oracle.

Name is provisional. The namespace is `cascade` throughout, so renaming is a
mechanical change.

## Why

Vector network analyzers are calibrated against standards whose properties are
known imperfectly, and the measurement is affected by connectors, cables, drift and
noise. A bare S-parameter plot hides all of that. Existing tools that do account
for it are either closed and Windows-only (METAS VNA Tools, NIST MUF) or ignore
uncertainty altogether (scikit-rf, libvna).

`cascade` aims at the gap:

1. **Correlation is the model, not a feature.** A measurement is a set of
   uncertain *input quantities* (standard definitions, connector repeatability,
   drift, noise) propagated through the calibration. Two outputs are correlated
   when they share inputs — across ports, and across frequency. That is what makes
   the uncertainty of a band average, a group delay or a fitted parameter
   computable at all.
2. **The budget is an output, not a report.** Every result carries a
   decomposition by influence: which physical effect dominates, and where.
3. **Cross-platform and embeddable.** C++20, Eigen, no runtime, no Windows. Usable
   from a CLI, a GUI, or another program.
4. **Verification is part of the product.** Golden datasets, parity tests against
   an independent implementation, and published acceptance numbers.

## Design principles

- **The model is written once and instantiated twice.** Every measurement model is
  a template over its scalar type: `double` for values, a forward-mode dual for
  sensitivities. Adding an influence is a one-line change; the Jacobian comes out
  mechanically. Hand-derived closed-form sensitivities are deliberately avoided.
- **Covariance is the central data structure.** Blocked by influence, sparse over
  frequency. Types come before algorithms.
- **Values, not state.** Models are pure functions of their inputs; there is no
  hidden mutable calibration state to get out of step with the data.
- **No silent fallbacks.** An unknown standard model, distribution or file variant
  is an error, never a default.

## Scope

See `docs/scope.md`. In short: one-port SOL and two-port SOLT first, with full
budgets and real-data parity. TRL, multiport, waveguide, mixers, noise parameters,
de-embedding and any GUI come later, if at all.

## Layout

```
include/cascade/   public headers (interface only, no implementation yet)
src/               implementation
tests/             unit, parity and acceptance tests
docs/              scope, design, verification, decision records
fixtures/          test data — provenance and licence rules apply, read the README
reference/         (planned) the Python prototype used as the oracle
```

## Building

Requires a C++20 compiler, CMake ≥ 3.24 and Eigen ≥ 3.4.

```sh
cmake --preset dev          # ninja, debug, tests on
cmake --build --preset dev
ctest --preset dev
```

Installed with `cmake --install build/dev`; headers go to
`include/cascade`, library to `lib`.

## Licence

Apache-2.0 — see `LICENSE`. Chosen for the explicit patent grant, which matters for
a tool intended to be adopted by laboratories.

Third-party data is **not** covered: the fixtures directory documents where each
dataset came from and what may be committed.
