// cascade — METAS .sdatb reading and writing
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <string>
#include <vector>

#include "cascade/matrix.hpp"
#include "cascade/uncertainty.hpp"

namespace cascade::io {

/// S-parameter data with its full covariance, as stored by METAS VNA Tools.
///
/// Why this format first: it is the only widely used one that carries
/// uncertainties *with dependencies* (Touchstone and CITI do not), and reading it
/// is what makes incremental adoption possible — a laboratory can keep its
/// archive and use this library alongside the tool that produced it.
///
/// Two layouts exist in the wild (see the published "Data Formats" document,
/// section 2):
///
///   * version 1 — gzip, one complex number per value, encoded as
///     `[int32 version][double re][double im]` when the data carries no
///     uncertainty. This is what raw measurements look like.
///   * version 2 — plain binary, a flat vector of values followed by a table of
///     named input distributions and one dependency list per value. Values are
///     `value + sum_k jacobi_k * u_k`, so the file holds the complete covariance.
///
/// Parsing notes worth keeping in the implementation:
///   * strings and section lengths are 7-bit encoded (as .NET writes them);
///   * dependency pointers are relative to the current value index;
///   * an unknown distribution type must raise — defaulting silently mis-scales
///     every uncertainty downstream;
///   * assert the parse consumes the file exactly. That check is what
///     distinguishes "this file uses the other layout" from "my parser is wrong".
struct SData {
    Grid grid;
    std::vector<Port> ports;
    ReferenceImpedance reference;
    std::vector<S2> values;  ///< one entry per frequency
    Covariance covariance;   ///< full, by influence
    int format_version{0};
};

/// Read one .sdatb file.
///
/// TODO(v0.1): port the working parser from the prototype (which is byte-exact on
/// the shipped METAS examples). Throw on malformed input; never return partially
/// populated data.
[[nodiscard]] SData read_sdatb(const std::string& path);

/// Write one .sdatb file, version 2.
///
/// TODO(v0.2): needed to round-trip results back to other tools. Writing rather
/// than only reading is also the cheapest way to prove the reader is correct:
/// read, write, read again, compare covariance rather than just values.
void write_sdatb(const std::string& path, const SData& data);

}  // namespace cascade::io
