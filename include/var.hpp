#pragma once
#include "tape.hpp"
#include <cmath>
constexpr double PI = 3.14159265358979323846;

// A Var is a number that also remembers its position on the tape,
// so every operation done with it gets recorded automatically.
class Var {
public:
    double value;
    int index;

    Var(double v) : value(v), index(tape().push_leaf()) {}
    Var(double v, int idx) : value(v), index(idx) {}
};

inline Var operator+(const Var& a, const Var& b) {
    double v = a.value + b.value;
    int idx = tape().push_binary(a.index, 1.0, b.index, 1.0);
    return Var(v, idx);
}

inline Var operator-(const Var& a, const Var& b) {
    double v = a.value - b.value;
    int idx = tape().push_binary(a.index, 1.0, b.index, -1.0);
    return Var(v, idx);
}

// product rule: d(a*b) = b*da + a*db
inline Var operator*(const Var& a, const Var& b) {
    double v = a.value * b.value;
    int idx = tape().push_binary(a.index, b.value, b.index, a.value);
    return Var(v, idx);
}

// quotient rule
inline Var operator/(const Var& a, const Var& b) {
    double v = a.value / b.value;
    double w1 = 1.0 / b.value;
    double w2 = -a.value / (b.value * b.value);
    int idx = tape().push_binary(a.index, w1, b.index, w2);
    return Var(v, idx);
}

inline Var operator-(const Var& a) {
    double v = -a.value;
    int idx = tape().push_unary(a.index, -1.0);
    return Var(v, idx);
}

inline Var exp(const Var& x) {
    double v = std::exp(x.value);
    int idx = tape().push_unary(x.index, v);
    return Var(v, idx);
}

inline Var log(const Var& x) {
    double v = std::log(x.value);
    int idx = tape().push_unary(x.index, 1.0 / x.value);
    return Var(v, idx);
}

inline Var sqrt(const Var& x) {
    double v = std::sqrt(x.value);
    int idx = tape().push_unary(x.index, 1.0 / (2.0 * v));
    return Var(v, idx);
}

// erf shows up in the normal distribution's CDF, which Black-Scholes needs
inline Var erf(const Var& x) {
    double v = std::erf(x.value);
double d = (2.0 / std::sqrt(PI)) * std::exp(-x.value * x.value);    int idx = tape().push_unary(x.index, d);
    return Var(v, idx);
}

// convenience so we can write things like sigma * 0.5 or 1.0 - x
inline Var operator+(const Var& a, double b) { return a + Var(b); }
inline Var operator+(double a, const Var& b) { return Var(a) + b; }
inline Var operator-(const Var& a, double b) { return a - Var(b); }
inline Var operator-(double a, const Var& b) { return Var(a) - b; }
inline Var operator*(const Var& a, double b) { return a * Var(b); }
inline Var operator*(double a, const Var& b) { return Var(a) * b; }
inline Var operator/(const Var& a, double b) { return a / Var(b); }
inline Var operator/(double a, const Var& b) { return Var(a) / b; }