// cascade — scalar polymorphism
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <concepts>
#include <cstddef>

namespace cascade {

/// Everything in this library is written once, as a template over its scalar
/// type, and instantiated twice: with `double` to compute values, and with a
/// forward-mode dual to compute sensitivities with respect to the measurement
/// inputs.
///
/// That is the central design decision. It means an influence can be added to a
/// measurement model in one line and its Jacobian column appears without
/// hand-derived algebra — the same property that makes the Python prototype
/// cheap to extend. Anything that breaks this property (special-casing a type,
/// returning `double` from a model) is a design error, not a shortcut.
///
/// Requirements on a scalar: real-valued arithmetic only. Complex quantities are
/// represented as pairs of scalars (see `Complex`), never as `std::complex`,
/// because a dual number is not an arithmetic type and would be rejected.
template<class T>
concept Scalar = std::regular<T> && requires(T a, T b, double d) {
    { a + b } -> std::convertible_to<T>;
    { a - b } -> std::convertible_to<T>;
    { a * b } -> std::convertible_to<T>;
    { a / b } -> std::convertible_to<T>;
    { -a } -> std::convertible_to<T>;
    { a += b } -> std::same_as<T&>;
    { a -= b } -> std::same_as<T&>;
    { a *= b } -> std::same_as<T&>;
    { a /= b } -> std::same_as<T&>;
    { T{d} } -> std::same_as<T>;
};

/// A complex number over an arbitrary scalar.
///
/// `std::complex<double>` cannot carry dual numbers, so the library carries its
/// own pair. Layout is two scalars; accessors keep it usable as a value type.
template<Scalar T>
struct Complex {
    T re{};
    T im{};

    constexpr Complex() = default;
    constexpr Complex(T real, T imag) : re(real), im(imag) {}

    [[nodiscard]] constexpr Complex operator+(const Complex& o) const {
        return {re + o.re, im + o.im};
    }
    [[nodiscard]] constexpr Complex operator-(const Complex& o) const {
        return {re - o.re, im - o.im};
    }
    [[nodiscard]] constexpr Complex operator-() const { return {-re, -im}; }
    [[nodiscard]] constexpr Complex operator*(const Complex& o) const {
        return {re * o.re - im * o.im, re * o.im + im * o.re};
    }
    [[nodiscard]] constexpr Complex operator/(const Complex& o) const {
        const T d = o.re * o.re + o.im * o.im;
        return {(re * o.re + im * o.im) / d, (im * o.re - re * o.im) / d};
    }
    constexpr Complex& operator+=(const Complex& o) { return *this = *this + o; }
    constexpr Complex& operator-=(const Complex& o) { return *this = *this - o; }
    constexpr Complex& operator*=(const Complex& o) { return *this = *this * o; }
    constexpr Complex& operator/=(const Complex& o) { return *this = *this / o; }
    [[nodiscard]] constexpr bool operator==(const Complex&) const = default;
};

using ComplexDouble = Complex<double>;

/// Forward-mode dual number carrying N partial derivatives.
///
/// Not implemented yet. The intended shape: `value` plus `grad[k] = d/dx_k`,
/// with arithmetic that applies the chain rule. N is a compile-time constant for
/// a given model (30 for two-port SOLT, 12 for one-port SOL) so a Jacobian is an
/// N-fold vector of derivative arrays and no dynamic allocation is needed.
///
/// TODO(v0.1): implement, with a test that compares it against central finite
/// differences on the calibration model (target: 1e-8 relative agreement).
template<class T, std::size_t N>
class Dual;

}  // namespace cascade
