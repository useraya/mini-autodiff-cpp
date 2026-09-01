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
// if sigma changed? if r changed? if T changed? All from this one function.
Var call_price(const Var& S, double K, const Var& r, const Var& sigma, const Var& T) {
    Var d1 = (log(S / K) + (r + sigma * sigma * 0.5) * T) / (sigma * sqrt(T));
    Var d2 = d1 - sigma * sqrt(T);
    Var price = S * norm_cdf(d1) - K * exp(-1.0 * r * T) * norm_cdf(d2);
    return price;
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

    // Wrap the inputs we want derivatives for as Var (these become
    // "leaves" on the tape). K is left as a plain double since we're
    // not asking for a derivative with respect to it here.
    Var S(S_val);
    Var r(r_val);
    Var sigma(sigma_val);
    Var T(T_val);

    Var price = call_price(S, K_val, r, sigma, T);

    // one backward pass gives us the sensitivity to every input at once
    std::vector<double> adj = tape().backward(price.index);

    double delta_ad = adj[S.index];
    double vega_ad   = adj[sigma.index];
    double rho_ad    = adj[r.index];
    double theta_ad  = -adj[T.index]; // theta is usually quoted as -d(price)/dT

    // closed-form formulas, computed independently, to check against
    double d1 = (std::log(S_val / K_val) + (r_val + sigma_val * sigma_val / 2.0) * T_val)
                / (sigma_val * std::sqrt(T_val));
    double d2 = d1 - sigma_val * std::sqrt(T_val);

    double delta_formula = norm_cdf_d(d1);
    double vega_formula  = S_val * norm_pdf(d1) * std::sqrt(T_val);
    double rho_formula   = K_val * T_val * std::exp(-r_val * T_val) * norm_cdf_d(d2);
    double theta_formula = -(S_val * norm_pdf(d1) * sigma_val) / (2.0 * std::sqrt(T_val))
                            - r_val * K_val * std::exp(-r_val * T_val) * norm_cdf_d(d2);

    std::cout << "Call price = " << price.value << "\n\n";

    std::cout << "Delta  (AD) = " << delta_ad << "   (formula) = " << delta_formula << "\n";
    std::cout << "Vega   (AD) = " << vega_ad  << "   (formula) = " << vega_formula  << "\n";
    std::cout << "Rho    (AD) = " << rho_ad   << "   (formula) = " << rho_formula   << "\n";
    std::cout << "Theta  (AD) = " << theta_ad << "   (formula) = " << theta_formula << "\n";

    return 0;
}