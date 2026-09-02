#pragma once
#include "var2.hpp"

namespace ad2 {

// This is the exact same formula as include/black_scholes.hpp, just
// written generically. It doesn't know or care whether T is a plain
// double or a Dual number -- the same code path computes an ordinary
// price when T=double, and gives us second-order information for free
// when T=Dual. That reuse is the actual point of writing it this way.
template <typename T>
inline Var<T> norm_cdf(const Var<T>& x) {
    return 0.5 * (1.0 + erf(x / std::sqrt(2.0)));
}

template <typename T>
inline Var<T> call_price(const Var<T>& S, const Var<T>& K, const Var<T>& r,
                          const Var<T>& sigma, const Var<T>& T_) {
    Var<T> d1 = (log(S / K) + (r + sigma * sigma * 0.5) * T_) / (sigma * sqrt(T_));
    Var<T> d2 = d1 - sigma * sqrt(T_);
    return S * norm_cdf(d1) - K * exp(-1.0 * r * T_) * norm_cdf(d2);
}

} // namespace ad2