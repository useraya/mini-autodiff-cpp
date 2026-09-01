#include <iostream>
#include <cmath>
#include "var.hpp"

// Standard normal CDF, built out of erf, using our Var type
Var norm_cdf(const Var& x) {
    return 0.5 * (1.0 + erf(x / std::sqrt(2.0)));
}

// The Black-Scholes call price formula, written using Var instead of double.
// Because it's built from Var, every step gets recorded on the tape,
// and we can later ask: how much would the price change if S changed?
// if sigma changed? if r changed? if T changed? if K changed? All from
// this one function, in one backward pass.
Var call_price(const Var& S, const Var& K, const Var& r, const Var& sigma, const Var& T) {
    Var d1 = (log(S / K) + (r + sigma * sigma * 0.5) * T) / (sigma * sqrt(T));
    Var d2 = d1 - sigma * sqrt(T);
    Var price = S * norm_cdf(d1) - K * exp(-1.0 * r * T) * norm_cdf(d2);
    return price;
}

// Put price is the mirror formula: same d1, d2, flipped signs and order.
// A put gains value as the stock falls, which is why S and K trade places.
Var put_price(const Var& S, const Var& K, const Var& r, const Var& sigma, const Var& T) {
    Var d1 = (log(S / K) + (r + sigma * sigma * 0.5) * T) / (sigma * sqrt(T));
    Var d2 = d1 - sigma * sqrt(T);
    Var price = K * exp(-1.0 * r * T) * norm_cdf(-1.0 * d2) - S * norm_cdf(-1.0 * d1);
    return price;
}

// Gamma is a second derivative (the derivative of Delta), so one backward
// pass can't hand it to us directly. Instead: get Delta via AD at S,
// get Delta via AD again at S nudged slightly, and finite-difference those
// two AD-computed Delta values. This is more accurate than bumping the
// price directly, since each Delta itself is exact, not approximated.
double call_delta_ad(double S_val, double K_val, double r_val, double sigma_val, double T_val) {
    tape().clear();
    Var S(S_val), K(K_val), r(r_val), sigma(sigma_val), T(T_val);
    Var price = call_price(S, K, r, sigma, T);
    std::vector<double> adj = tape().backward(price.index);
    return adj[S.index];
}

double call_gamma(double S_val, double K_val, double r_val, double sigma_val, double T_val) {
    double h = 0.01;
    double delta_up   = call_delta_ad(S_val + h, K_val, r_val, sigma_val, T_val);
    double delta_down = call_delta_ad(S_val - h, K_val, r_val, sigma_val, T_val);
    return (delta_up - delta_down) / (2.0 * h);
}

// --- Below: plain-double versions of the textbook closed-form Greeks,
// used only to check that our AD answers are correct. ---

double norm_pdf(double x) {
    return std::exp(-x * x / 2.0) / std::sqrt(2.0 * PI);
}

double norm_cdf_d(double x) {
    return 0.5 * (1.0 + std::erf(x / std::sqrt(2.0)));
}

int main() {
    double S_val = 100.0;   // stock price
    double K_val = 100.0;   // strike price
    double r_val = 0.05;    // risk-free rate
    double sigma_val = 0.2; // volatility
    double T_val = 1.0;     // time to expiry (years)

    // --- CALL ---
    tape().clear();
    Var S(S_val), K(K_val), r(r_val), sigma(sigma_val), T(T_val);
    Var call = call_price(S, K, r, sigma, T);
    std::vector<double> adj_call = tape().backward(call.index);

    double call_delta = adj_call[S.index];
    double call_vega   = adj_call[sigma.index];
    double call_rho    = adj_call[r.index];
    double call_theta  = -adj_call[T.index];
    double call_dK      = adj_call[K.index]; // sensitivity to strike, "free" bonus
    double gamma        = call_gamma(S_val, K_val, r_val, sigma_val, T_val);

    // --- PUT (separate tape, separate backward pass) ---
    tape().clear();
    Var S2(S_val), K2(K_val), r2(r_val), sigma2(sigma_val), T2(T_val);
    Var put = put_price(S2, K2, r2, sigma2, T2);
    std::vector<double> adj_put = tape().backward(put.index);

    double put_delta = adj_put[S2.index];
    double put_vega   = adj_put[sigma2.index];
    double put_rho    = adj_put[r2.index];
    double put_theta  = -adj_put[T2.index];

    // closed-form formulas, computed independently, to check the call Greeks
    double d1 = (std::log(S_val / K_val) + (r_val + sigma_val * sigma_val / 2.0) * T_val)
                / (sigma_val * std::sqrt(T_val));
    double d2 = d1 - sigma_val * std::sqrt(T_val);

    double delta_formula = norm_cdf_d(d1);
    double vega_formula  = S_val * norm_pdf(d1) * std::sqrt(T_val);
    double rho_formula   = K_val * T_val * std::exp(-r_val * T_val) * norm_cdf_d(d2);
    double theta_formula = -(S_val * norm_pdf(d1) * sigma_val) / (2.0 * std::sqrt(T_val))
                            - r_val * K_val * std::exp(-r_val * T_val) * norm_cdf_d(d2);
    double gamma_formula = norm_pdf(d1) / (S_val * sigma_val * std::sqrt(T_val));

    std::cout << "Call price = " << call.value << "\n";
    std::cout << "Put price  = " << put.value  << "\n\n";

    std::cout << "-- Call Greeks --\n";
    std::cout << "Delta (AD) = " << call_delta << "   (formula) = " << delta_formula << "\n";
    std::cout << "Vega  (AD) = " << call_vega  << "   (formula) = " << vega_formula  << "\n";
    std::cout << "Rho   (AD) = " << call_rho   << "   (formula) = " << rho_formula   << "\n";
    std::cout << "Theta (AD) = " << call_theta << "   (formula) = " << theta_formula << "\n";
    std::cout << "Gamma (AD) = " << gamma      << "   (formula) = " << gamma_formula << "\n";
    std::cout << "dPrice/dK  = " << call_dK    << "   (sensitivity to strike)\n\n";

    std::cout << "-- Put Greeks --\n";
    std::cout << "Delta (AD) = " << put_delta << "\n";
    std::cout << "Vega  (AD) = " << put_vega  << "\n";
    std::cout << "Rho   (AD) = " << put_rho   << "\n";
    std::cout << "Theta (AD) = " << put_theta << "\n";

    return 0;
}