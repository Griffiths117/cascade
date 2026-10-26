// cascade — calibration
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <string>
#include <vector>

#include "cascade/matrix.hpp"
#include "cascade/standard.hpp"

namespace cascade {

/// Calibration types. v0.1 covers the two that a first implementation can be
/// verified against real data; the rest are named here so the interface does not
/// have to change when they arrive.
enum class CalibrationKind {
    one_port_sol,
    two_port_solt,
    // not implemented yet, listed for interface stability:
    trl,
    lrl,
    multiline_trl,
    unknown_thru,
};

/// The solved error terms, as the two error boxes of the 8-term model.
///
/// The model is the cascade  M = star(star(A, S), B)  where A is the port-1 box,
/// B the port-2 box, S the device under test and `star` the S-parameter cascade.
/// Keeping the result in box form (rather than as a flat list of "12 terms") is
/// what makes correction a matrix operation and what keeps the one-port case a
/// special case instead of a separate code path.
///
/// Gauge: the boxes are only defined up to a scaling of A against B, so one
/// entry is fixed. This library fixes `A(0,1) = 1` (in METAS VNA Tools notation
/// E10 = 1), a choice worth stating because it is rarely written down and makes
/// results comparable with theirs.
template<int Ports>
struct ErrorTerms {
    SMatrixD<Ports> box_a;
    SMatrixD<Ports> box_b;
};

/// One calibration: measured standards in, error terms out.
///
/// The solve is *linear in the error terms* for SOLT (three equations per port
/// plus one thru equation), which is what keeps it cheap and keeps its
/// sensitivities exact. Do not "improve" it into a nonlinear least-squares fit
/// for the standard set: the linear form is the reason a Jacobian column can be
/// obtained mechanically.
///
/// The thru is special: its definition enters the estimator as an *assumed*
/// S21. A definition error is therefore an input quantity, not a measurement
/// perturbation. Modelled the other way round, the parameter never appears in
/// the Jacobian and the budget silently reports a confident 0.00 %.
class Calibration {
public:
    Calibration(CalibrationKind kind, std::vector<StandardDefinition> standards);

    [[nodiscard]] CalibrationKind kind() const noexcept;

    /// Solve for the error terms from raw measurements of the standards.
    ///
    /// `measured` is indexed by frequency, then by standard in the same order as
    /// the definitions supplied at construction.
    ///
    /// TODO(v0.1): implement one-port SOL and two-port SOLT. Acceptance: a
    /// noise-free round trip recovers the injected device to <= 1e-13 on all
    /// four S-parameters.
    template<class S>
    [[nodiscard]] ErrorTerms<2>
    solve(const Grid& grid, const std::vector<std::vector<SMatrix<S, 2>>>& measured) const;

private:
    CalibrationKind kind_;
    std::vector<StandardDefinition> standards_;
};

/// Apply error terms to a raw measurement, recovering the device's S-parameters.
///
/// Implemented through T-matrices (T_device = inv(T_A) * T_raw * inv(T_B)), which
/// is exact and cheap, but is undefined for a device with zero transmission. The
/// cascade form used for *modelling* raw data has no such restriction; only this
/// inversion does, and the one-port case avoids it entirely.
///
/// TODO(v0.1): implement; document the S21 == 0 case rather than letting a
/// division by zero produce infinities.
template<class S>
[[nodiscard]] SMatrix<S, 2> correct(const ErrorTerms<2>& terms, const SMatrix<S, 2>& raw);

}  // namespace cascade
