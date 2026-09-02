#pragma once
#include <cmath>

// A Dual number holds two things: the actual value, and how fast
// that value is changing (its derivative), at the same time.
// Every operation below updates both, so by the end of a calculation
// we get the derivative "for free", without guessing or re-running anything.
struct Dual {
    double value;
    double deriv;

    Dual(double v, double d = 0.0) : value(v), deriv(d) {}
};

// Addition: (a+b)' = a' + b'
inline Dual operator+(const Dual& a, const Dual& b) {
    return Dual(a.value + b.value, a.deriv + b.deriv);
}

// Subtraction: (a-b)' = a' - b'
inline Dual operator-(const Dual& a, const Dual& b) {
    return Dual(a.value - b.value, a.deriv - b.deriv);
}

// Multiplication uses the product rule: (a*b)' = a'*b + a*b'
inline Dual operator*(const Dual& a, const Dual& b) {
    return Dual(a.value * b.value, a.deriv * b.value + a.value * b.deriv);
}

// Division uses the quotient rule: (a/b)' = (a'*b - a*b') / b^2
inline Dual operator/(const Dual& a, const Dual& b) {
    double val = a.value / b.value;
    double der = (a.deriv * b.value - a.value * b.deriv) / (b.value * b.value);
    return Dual(val, der);
}

// sin(x)' = cos(x) * x'
inline Dual sin(const Dual& x) {
    return Dual(std::sin(x.value), std::cos(x.value) * x.deriv);
}

// cos(x)' = -sin(x) * x'
inline Dual cos(const Dual& x) {
    return Dual(std::cos(x.value), -std::sin(x.value) * x.deriv);
}

// exp(x)' = exp(x) * x'
inline Dual exp(const Dual& x) {
    double e = std::exp(x.value);
    return Dual(e, e * x.deriv);
}

// log(x)' = x' / x
inline Dual log(const Dual& x) {
    return Dual(std::log(x.value), x.deriv / x.value);
}

// sqrt(x)' = x' / (2 * sqrt(x))
inline Dual sqrt(const Dual& x) {
    double s = std::sqrt(x.value);
    return Dual(s, x.deriv / (2.0 * s));
}
// -x
inline Dual operator-(const Dual& x) {
    return Dual(-x.value, -x.deriv);
}

// erf(x)' = (2/sqrt(pi)) * exp(-x^2) * x'
inline Dual erf(const Dual& x) {
    constexpr double PI_D = 3.14159265358979323846;
    double e = (2.0 / std::sqrt(PI_D)) * std::exp(-x.value * x.value);
    return Dual(std::erf(x.value), e * x.deriv);
}