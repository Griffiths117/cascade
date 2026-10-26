# Contributing

## The rules that are not negotiable

These exist because each one was learned the hard way, and breaking one produces a
plausible-looking wrong answer rather than a failure.

1. **Models take a scalar template parameter and never return `double`.** A model
   that returns a concrete type cannot be instantiated over the dual, so its
   sensitivities have to be derived by hand — which is the cost this library exists
   to avoid. Same for special-casing a type anywhere in `include/cascade/`.
2. **No silent fallbacks.** Unknown calibration standard, unknown file variant,
   unknown distribution: throw. A default that guesses turns a malformed input into
   a wrong number.
3. **A standard's definition is an input quantity, not a perturbation.** If the
   estimator consumes an assumed value, that value must be a model input, or its
   Jacobian column is zero and the budget will report a confident `0.00 %`.
4. **Verify each claim with a number.** If a change affects a result, the
   corresponding test in `docs/verification.md` reports the measured value. "Tests
   pass" is not a verification report.
5. **No allocation in value types, no unordered iteration where it affects a
   result.** Results must be reproducible bit for bit.

## Workflow

```sh
cmake --preset dev && cmake --build --preset dev && ctest --preset dev
cmake --preset sanitize && cmake --build --preset sanitize && ctest --preset sanitize
```

Formatting is `clang-format` with the checked-in `.clang-format`; static analysis is
`clang-tidy` with the checked-in `.clang-tidy`. Both are advisory locally and
enforced in CI.

## Adding a calibration type

1. Add the enum value in `calibration.hpp`.
2. Implement the solve as a template over the scalar. If the derivation requires an
   eigenvalue decomposition, stop and write an ADR first — the branch selection is
   the hard part, not the algebra.
3. Add the round-trip test before the uncertainty test. A calibration that does not
   recover a known device exactly has nothing to say about uncertainty.
4. Add its influences to the budget and state explicitly which are omitted.

## Adding an influence

One line in the model plus one entry in the influence list. If it needs more than
that, the model is structured wrongly — say so in an ADR rather than working around
it.

## Data

Never commit vendor data. See `fixtures/README.md`; the rule is that the repository
holds our data and hashes, never somebody else's files.
