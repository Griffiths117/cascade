// cascade — influences, covariance and budgets
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <string>
#include <vector>

#include "cascade/matrix.hpp"
#include "cascade/scalar.hpp"

namespace cascade {

/// Probability distribution of an input quantity.
///
/// Only the shapes a calibration actually uses are named. As in the file formats,
/// the standard uncertainty of a uniform input is (b - a) / sqrt(12); the reader
/// converts, the model never sees the raw bounds.
struct Distribution {
    enum class Kind { normal, uniform };
    Kind kind{Kind::normal};
    double mean{0.0};
    double spread{0.0};  ///< sigma for normal, half-width for uniform
};

/// One physical cause of uncertainty.
///
/// Influences are the unit of both modelling and reporting: a budget is a
/// per-influence decomposition of the output variance, so the granularity here
/// decides what the user can act on. Practical guidance from the prototype:
/// connector repeatability is per connection, drift is per error-term and
/// correlated over time, standard definitions are per standard, and noise is
/// level dependent (an additive floor plus a multiplicative trace term).
struct Influence {
    std::string id;           ///< stable machine name
    std::string description;  ///< human-readable, e.g. "connector repeatability, port 1"
    std::string unit;         ///< "1", "m", "dB", "deg"
    Distribution distribution;
};

/// A covariance matrix that remembers which influence produced which part.
///
/// This is the central data structure of the library, and the reason it exists at
/// all. A plain dense covariance would work for per-point error bars and then fail
/// on the questions people actually ask, because:
///
///   * a budget needs the decomposition by influence, not just the total;
///   * measurements share influences across ports and across *frequency*, so the
///     interesting blocks are off-diagonal in both.
///
/// The prototype learned this the hard way: adding cross-frequency correlation
/// after the fact changed the uncertainty of a band average by a factor of 7.6
/// while leaving every per-point error bar identical.
///
/// TODO(v0.1): representation. Intended shape: one sparse block per influence,
/// with the frequency axis as the batch dimension, plus an accessor that assembles
/// the dense block for a requested (frequency, port) set.
class Covariance {
public:
    Covariance() = default;
    explicit Covariance(std::vector<Influence> influences);

    [[nodiscard]] const std::vector<Influence>& influences() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;

private:
    std::vector<Influence> influences_;
};

/// Where the output variance came from.
///
/// A result carries this, always. Reporting an uncertainty without saying which
/// influence produced it leaves the user unable to decide whether to buy a better
/// VNA, a better calibration kit, or simply take more care with the connectors.
struct Budget {
    struct Term {
        std::string influence_id;
        double variance{0.0};
    };
    std::vector<Term> terms;

    [[nodiscard]] double total_variance() const;
    /// Share of the total in [0, 1]; zero when the total is zero.
    [[nodiscard]] double share(const Term& term) const;
};

/// How well first-order propagation describes a given result.
///
/// Linear propagation under-predicts where the output's derivative vanishes — a
/// rotating S21 at a phase extremum is the concrete case, where the prototype saw
/// the ratio fall to 0.76 and coverage to 93 %. The second-order (curvature) term
/// repairs it, at roughly thirty times the cost of the Jacobian.
///
/// Rather than hiding that, every result can carry the diagnostic.
struct LinearisationFlag {
    double sigma_linear{0.0};
    double sigma_monte_carlo{0.0};                   ///< 0 when not evaluated
    [[nodiscard]] bool is_suspect() const noexcept;  ///< true when the ratio leaves 1 +/- 5 %
};

/// Propagate a covariance through a model's sensitivities.
///
/// `sigma_out = sum_k J_k Sigma_k J_k^T`, once per influence, which gives the
/// budget for free.
///
/// TODO(v0.1): implement for a caller-supplied Jacobian; Monte Carlo and the
/// curvature term follow once the linear path is verified against the reference.
class Propagator {
public:
    [[nodiscard]] static Budget budget_from_jacobian(const Covariance& covariance,
                                                     const std::vector<double>& jacobian,
                                                     std::size_t outputs);
};

}  // namespace cascade
