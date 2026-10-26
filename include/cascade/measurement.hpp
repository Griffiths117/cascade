// cascade — measurements and the journal
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "cascade/matrix.hpp"

namespace cascade {

/// One raw measurement: what the instrument reported, before any calibration.
///
/// Raw data is stored as measured and never overwritten. A calibration is a
/// *view* over measurements, so the same raw file can be re-processed with
/// corrected standard definitions without re-measuring — which is precisely the
/// workflow a laboratory needs when a kit is re-characterised years later.
struct Measurement {
    std::string id;                  ///< file or record identifier
    std::int64_t timestamp_unix{0};  ///< seconds since epoch, UTC
    int connection_id{0};            ///< increments on every reconnection
    std::vector<Port> ports;
    std::vector<S2> raw;  ///< one entry per frequency, matches Grid order
};

/// The measurement journal: a timestamped record of what was connected when.
///
/// Drift and connector repeatability are not properties of a number, they are
/// properties of a *sequence* of events. Without the journal there is no honest
/// way to decide which measurements share a connection (correlated repeatability)
/// and which are separated by a cable move (independent), so the journal is a
/// prerequisite for the correlation model rather than bookkeeping.
class Journal {
public:
    Journal() = default;

    void add(Measurement measurement);
    [[nodiscard]] const std::vector<Measurement>& measurements() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;

    /// Measurements taken between two events, in order. Used to define the
    /// influence blocks for drift: the longer the interval, the more drift
    /// accumulates, which is why the model needs time as an input.
    [[nodiscard]] std::vector<const Measurement*> between(std::int64_t from_unix,
                                                          std::int64_t to_unix) const;

private:
    std::vector<Measurement> measurements_;
};

}  // namespace cascade
