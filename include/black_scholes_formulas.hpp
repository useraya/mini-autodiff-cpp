#pragma once
#include <cmath>

constexpr double PI_CONST = 3.14159265358979323846;

inline double norm_pdf_d(double x) {
    return std::exp(-x * x / 2.0) / std::sqrt(2.0 * PI_CONST);
}

inline double norm_cdf_d(double x) {
    return 0.5 * (1.0 + std::erf(x / std::sqrt(2.0)));
}

struct CallGreeksFormula {
    double price, delta, vega, rho, theta, gamma;
};

// The textbook closed-form Black-Scholes call price and Greeks.
// Used only to check the AD engine's answers, never used to price
// anything on its own in this project.
inline CallGreeksFormula call_greeks_formula(double S, double K, double r,
                                              double sigma, double T) {
    double d1 = (std::log(S / K) + (r + sigma * sigma / 2.0) * T) / (sigma * std::sqrt(T));
    double d2 = d1 - sigma * std::sqrt(T);

    CallGreeksFormula g;
    g.price = S * norm_cdf_d(d1) - K * std::exp(-r * T) * norm_cdf_d(d2);
    g.delta = norm_cdf_d(d1);
    g.vega  = S * norm_pdf_d(d1) * std::sqrt(T);
    g.rho   = K * T * std::exp(-r * T) * norm_cdf_d(d2);
    g.theta = -(S * norm_pdf_d(d1) * sigma) / (2.0 * std::sqrt(T))
              - r * K * std::exp(-r * T) * norm_cdf_d(d2);
    g.gamma = norm_pdf_d(d1) / (S * sigma * std::sqrt(T));
    return g;
}