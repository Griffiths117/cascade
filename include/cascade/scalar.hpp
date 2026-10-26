// cascade — scalar polymorphism
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <array>
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
/// `value` is the primal value; `grad[k]` is d/dx_k. Arithmetic applies the
/// chain rule, so a model instantiated over `Dual` yields its Jacobian
/// mechanically. N is a compile-time constant for a given model.
template<class T, std::size_t N>
class Dual {
public:
    static_assert(N > 0, "a dual needs at least one derivative");

    T value{};
    std::array<T, N> grad{};

    constexpr Dual() = default;

    /// Primal constructor; derivatives default to zero.
    constexpr explicit Dual(T v) : value(v), grad{} {}

    constexpr Dual(T v, const std::array<T, N>& g) : value(v), grad(g) {}

    [[nodiscard]] constexpr T& operator[](std::size_t i) { return grad[i]; }
    [[nodiscard]] constexpr const T& operator[](std::size_t i) const { return grad[i]; }

    constexpr Dual& operator+=(const Dual& o) {
        value += o.value;
        for (std::size_t k = 0; k < N; ++k)
            grad[k] += o.grad[k];
        return *this;
    }
    constexpr Dual& operator-=(const Dual& o) {
        value -= o.value;
        for (std::size_t k = 0; k < N; ++k)
            grad[k] -= o.grad[k];
        return *this;
    }
    constexpr Dual& operator*=(const Dual& o) {
        // d(a*b) = da*b + a*db, evaluated at the current values.
        std::array<T, N> new_grad{};
        for (std::size_t k = 0; k < N; ++k) {
            new_grad[k] = grad[k] * o.value + value * o.grad[k];
        }
        value *= o.value;
        grad = new_grad;
        return *this;
    }
    constexpr Dual& operator/=(const Dual& o) {
        // d(a/b) = (da*b - a*db) / b^2
        std::array<T, N> new_grad{};
        for (std::size_t k = 0; k < N; ++k) {
            new_grad[k] = (grad[k] * o.value - value * o.grad[k]) / (o.value * o.value);
        }
        value /= o.value;
        grad = new_grad;
        return *this;
    }

    [[nodiscard]] constexpr bool operator==(const Dual&) const = default;
};

template<class T, std::size_t N>
[[nodiscard]] constexpr Dual<T, N> operator+(const Dual<T, N>& a, const Dual<T, N>& b) {
    Dual<T, N> r{a.value + b.value};
    for (std::size_t k = 0; k < N; ++k)
        r.grad[k] = a.grad[k] + b.grad[k];
    return r;
}

template<class T, std::size_t N>
[[nodiscard]] constexpr Dual<T, N> operator-(const Dual<T, N>& a, const Dual<T, N>& b) {
    Dual<T, N> r{a.value - b.value};
    for (std::size_t k = 0; k < N; ++k)
        r.grad[k] = a.grad[k] - b.grad[k];
    return r;
}

template<class T, std::size_t N>
[[nodiscard]] constexpr Dual<T, N> operator-(const Dual<T, N>& a) {
    Dual<T, N> r{-a.value};
    for (std::size_t k = 0; k < N; ++k)
        r.grad[k] = -a.grad[k];
    return r;
}

template<class T, std::size_t N>
[[nodiscard]] constexpr Dual<T, N> operator*(const Dual<T, N>& a, const Dual<T, N>& b) {
    Dual<T, N> r{a.value * b.value};
    for (std::size_t k = 0; k < N; ++k)
        r.grad[k] = a.grad[k] * b.value + a.value * b.grad[k];
    return r;
}

template<class T, std::size_t N>
[[nodiscard]] constexpr Dual<T, N> operator/(const Dual<T, N>& a, const Dual<T, N>& b) {
    Dual<T, N> r{a.value / b.value};
    for (std::size_t k = 0; k < N; ++k)
        r.grad[k] = (a.grad[k] * b.value - a.value * b.grad[k]) / (b.value * b.value);
    return r;
}

}  // namespace cascade
