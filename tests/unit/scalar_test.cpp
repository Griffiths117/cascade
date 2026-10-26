#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "cascade/scalar.hpp"

#include <cmath>

using namespace cascade;

TEST_CASE("Dual satisfies Scalar concept", "[unit][scalar]") {
    static_assert(Scalar<Dual<double, 3>>);
    SUCCEED();
}

TEST_CASE("Dual arithmetic and derivatives", "[unit][scalar]") {
    using D = Dual<double, 2>;
    D x{2.0};
    x[0] = 1.0;  // dx/dx0
    D y{3.0};
    y[1] = 1.0;  // dy/dx1

    D sum = x + y;
    REQUIRE(sum.value == 5.0);
    REQUIRE(sum[0] == 1.0);
    REQUIRE(sum[1] == 1.0);

    D prod = x * y;
    REQUIRE(prod.value == 6.0);
    REQUIRE(prod[0] == 3.0);  // d(xy)/dx0 = y
    REQUIRE(prod[1] == 2.0);  // d(xy)/dx1 = x

    D quot = x / y;
    REQUIRE_THAT(quot.value, Catch::Matchers::WithinAbs(2.0 / 3.0, 1e-15));
    REQUIRE_THAT(quot[0], Catch::Matchers::WithinAbs(1.0 / 3.0, 1e-15));
    REQUIRE_THAT(quot[1], Catch::Matchers::WithinAbs(-2.0 / 9.0, 1e-15));
}

TEST_CASE("Dual matches central differences", "[unit][scalar]") {
    // f(x0,x1) = x0*x0 + 2*x0*x1 + x1*x1  (a quadratic)
    auto f = [](double a, double b) { return a * a + 2.0 * a * b + b * b; };
    const double x0 = 1.3, x1 = -0.7;
    using D = Dual<double, 2>;
    D a{x0};
    a[0] = 1.0;
    D b{x1};
    b[1] = 1.0;
    // evaluate as dual by composing: need both variables in one expression
    D X{x0};
    X[0] = 1.0;
    D Y{x1};
    Y[1] = 1.0;
    D F = X * X + D{2.0} * X * Y + Y * Y;
    const double eps = 1e-6;
    const double d0 = (f(x0 + eps, x1) - f(x0 - eps, x1)) / (2 * eps);
    const double d1 = (f(x0, x1 + eps) - f(x0, x1 - eps)) / (2 * eps);
    REQUIRE_THAT(F[0], Catch::Matchers::WithinAbs(d0, 1e-8));
    REQUIRE_THAT(F[1], Catch::Matchers::WithinAbs(d1, 1e-8));
}
