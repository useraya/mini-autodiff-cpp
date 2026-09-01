#pragma once
#include "var.hpp"
#include <cmath>

// Standard normal CDF, built out of erf, using our Var type
inline Var norm_cdf(const Var& x) {
    return 0.5 * (1.0 + erf(x / std::sqrt(2.0)));
}

// Call price, written with Var so every step is recorded on the tape.
// This lets us get Delta, Vega, Rho, Theta, and sensitivity to K,
// all from one backward pass after calling this.
inline Var call_price(const Var& S, const Var& K, const Var& r, const Var& sigma, const Var& T) {
    Var d1 = (log(S / K) + (r + sigma * sigma * 0.5) * T) / (sigma * sqrt(T));
    Var d2 = d1 - sigma * sqrt(T);
    return S * norm_cdf(d1) - K * exp(-1.0 * r * T) * norm_cdf(d2);
}

// Put price: same d1, d2, flipped signs and order.
inline Var put_price(const Var& S, const Var& K, const Var& r, const Var& sigma, const Var& T) {
    Var d1 = (log(S / K) + (r + sigma * sigma * 0.5) * T) / (sigma * sqrt(T));
    Var d2 = d1 - sigma * sqrt(T);
    return K * exp(-1.0 * r * T) * norm_cdf(-1.0 * d2) - S * norm_cdf(-1.0 * d1);
}

// Runs the AD engine once and returns Delta, Vega, Rho, Theta, and
// sensitivity to K, for a call, at the given inputs.
struct CallGreeksAD {
    double price, delta, vega, rho, theta, dK;
};

inline CallGreeksAD call_greeks_ad(double S_val, double K_val, double r_val,
                                    double sigma_val, double T_val) {
    tape().clear();
    Var S(S_val), K(K_val), r(r_val), sigma(sigma_val), T(T_val);
    Var price = call_price(S, K, r, sigma, T);
    std::vector<double> adj = tape().backward(price.index);

    CallGreeksAD g;
    g.price = price.value;
    g.delta = adj[S.index];
    g.vega  = adj[sigma.index];
    g.rho   = adj[r.index];
    g.theta = -adj[T.index];
    g.dK    = adj[K.index];
    return g;
}

// Same, for a put.
struct PutGreeksAD {
    double price, delta, vega, rho, theta;
};

inline PutGreeksAD put_greeks_ad(double S_val, double K_val, double r_val,
                                  double sigma_val, double T_val) {
    tape().clear();
    Var S(S_val), K(K_val), r(r_val), sigma(sigma_val), T(T_val);
    Var price = put_price(S, K, r, sigma, T);
    std::vector<double> adj = tape().backward(price.index);

    PutGreeksAD g;
    g.price = price.value;
    g.delta = adj[S.index];
    g.vega  = adj[sigma.index];
    g.rho   = adj[r.index];
    g.theta = -adj[T.index];
    return g;
}

// Gamma via finite-differencing the AD-computed Delta (see README for why).
inline double call_gamma_ad(double S_val, double K_val, double r_val,
                             double sigma_val, double T_val) {
    double h = 0.01;
    double delta_up   = call_greeks_ad(S_val + h, K_val, r_val, sigma_val, T_val).delta;
    double delta_down = call_greeks_ad(S_val - h, K_val, r_val, sigma_val, T_val).delta;
    return (delta_up - delta_down) / (2.0 * h);
}