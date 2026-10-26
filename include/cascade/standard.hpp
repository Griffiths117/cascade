// cascade — calibration standards
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <string>
#include <vector>

#include "cascade/matrix.hpp"
#include "cascade/scalar.hpp"

namespace cascade {

/// How a calibration standard's reflection (or transmission) is defined.
///
/// The distinction between "what the standard physically is" and "what the
/// library assumes it is" is where most of the uncertainty in a VNA calibration
/// lives, so the two are kept separate in the API:
///
///   * `StandardModel` describes the *assumed* behaviour, parameterised by
///     quantities that carry uncertainties (offset length, fringe capacitance,
///     load residual).
///   * The difference between the assumed and the true behaviour is an
///     *influence*, and must be an explicit input to the model — see
///     `uncertainty.hpp`. A definition error is not a perturbation of the
///     measurement; it is an error in the number the estimator consumes.
enum class StandardModel {
    ideal_short,   ///< Gamma = -1
    ideal_open,    ///< Gamma = +1
    ideal_load,    ///< Gamma = 0
    offset_short,  ///< -exp(-2j*beta*l), l an uncertain length
    offset_open,   ///< +exp(-2j*beta*l)
    agilent,       ///< offset line cascaded with an L/C reflect part
    databased,     ///< explicitly supplied S-parameters (with uncertainties)
};

/// Parameter set for one calibration standard.
///
/// Deliberately a plain aggregate: it is a description, not behaviour, and it is
/// what a project file or a calibration certificate carries.
struct StandardDefinition {
    std::string id;  ///< short identifier, e.g. "short(f)"
    StandardModel model{StandardModel::ideal_short};
    std::vector<double> offset_poly;  ///< length or L/C polynomial coefficients, model-dependent
    std::string medium;               ///< "coaxial" for v0.1
    double z0_ohm{50.0};              ///< characteristic impedance of the offset line
};

/// The standard's assumed S-parameters at every frequency in the grid.
///
/// Instantiated with `double` for the calibration solve and with a dual to obtain
/// the sensitivities of the result to the definition parameters.
///
/// TODO(v0.1): implement ideal and offset models; `agilent` and `databased`
/// after the first parity milestone. Unknown `StandardModel` values must throw,
/// never fall back to ideal.
template<class S, int Ports>
[[nodiscard]] SMatrix<S, Ports> standard_response(const StandardDefinition& def, const Grid& grid);

}  // namespace cascade
