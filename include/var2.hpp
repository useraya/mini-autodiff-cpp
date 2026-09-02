#pragma once
#include "tape2.hpp"
#include "dual.hpp"
#include <cmath>

namespace ad2 {

template <typename T>
class Var {
public:
    T value;
    int index;

    Var(T v) : value(v), index(tape<T>().push_leaf()) {}
    Var(T v, int idx) : value(v), index(idx) {}
};

template <typename T>
inline Var<T> operator+(const Var<T>& a, const Var<T>& b) {
    T v = a.value + b.value;
    int idx = tape<T>().push_binary(a.index, T(1.0), b.index, T(1.0));
    return Var<T>(v, idx);
}

template <typename T>
inline Var<T> operator-(const Var<T>& a, const Var<T>& b) {
    T v = a.value - b.value;
    int idx = tape<T>().push_binary(a.index, T(1.0), b.index, T(-1.0));
    return Var<T>(v, idx);
}

template <typename T>
inline Var<T> operator*(const Var<T>& a, const Var<T>& b) {
    T v = a.value * b.value;
    int idx = tape<T>().push_binary(a.index, b.value, b.index, a.value);
    return Var<T>(v, idx);
}

template <typename T>
inline Var<T> operator/(const Var<T>& a, const Var<T>& b) {
    T v = a.value / b.value;
    T w1 = T(1.0) / b.value;
    T w2 = T(-1.0) * a.value / (b.value * b.value);
    int idx = tape<T>().push_binary(a.index, w1, b.index, w2);
    return Var<T>(v, idx);
}

template <typename T>
inline Var<T> operator-(const Var<T>& a) {
    T v = T(-1.0) * a.value;
    int idx = tape<T>().push_unary(a.index, T(-1.0));
    return Var<T>(v, idx);
}

// "using std::exp;" makes plain doubles fall back to the standard library,
// while unqualified exp(...) still finds our custom exp(Dual) via ADL
// when T is Dual. Same trick is used for log, sqrt, erf below.
template <typename T>
inline Var<T> exp(const Var<T>& x) {
    using std::exp;
    T v = exp(x.value);
    int idx = tape<T>().push_unary(x.index, v);
    return Var<T>(v, idx);
}

template <typename T>
inline Var<T> log(const Var<T>& x) {
    using std::log;
    T v = log(x.value);
    T w = T(1.0) / x.value;
    int idx = tape<T>().push_unary(x.index, w);
    return Var<T>(v, idx);
}

template <typename T>
inline Var<T> sqrt(const Var<T>& x) {
    using std::sqrt;
    T v = sqrt(x.value);
    T w = T(1.0) / (T(2.0) * v);
    int idx = tape<T>().push_unary(x.index, w);
    return Var<T>(v, idx);
}

template <typename T>
inline Var<T> erf(const Var<T>& x) {
    using std::erf;
    using std::exp;
    T v = erf(x.value);
    T pi_const = T(3.14159265358979323846);
    using std::sqrt;
    T w = (T(2.0) / sqrt(pi_const)) * exp(T(-1.0) * x.value * x.value);
    int idx = tape<T>().push_unary(x.index, w);
    return Var<T>(v, idx);
}

template <typename T>
inline Var<T> operator+(const Var<T>& a, double b) { return a + Var<T>(T(b)); }
template <typename T>
inline Var<T> operator+(double a, const Var<T>& b) { return Var<T>(T(a)) + b; }
template <typename T>
inline Var<T> operator-(const Var<T>& a, double b) { return a - Var<T>(T(b)); }
template <typename T>
inline Var<T> operator-(double a, const Var<T>& b) { return Var<T>(T(a)) - b; }
template <typename T>
inline Var<T> operator*(const Var<T>& a, double b) { return a * Var<T>(T(b)); }
template <typename T>
inline Var<T> operator*(double a, const Var<T>& b) { return Var<T>(T(a)) * b; }
template <typename T>
inline Var<T> operator/(const Var<T>& a, double b) { return a / Var<T>(T(b)); }
template <typename T>
inline Var<T> operator/(double a, const Var<T>& b) { return Var<T>(T(a)) / b; }

} // namespace ad2