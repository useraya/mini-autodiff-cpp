#include <iostream>
#include <cmath>
#include <random>
#include "var.hpp"

// Closed-form Black-Scholes, used ONLY as a sanity check here since this
// particular payoff (plain European call) happens to have a known formula.
// The Monte Carlo + AD method below doesn't need that formula to exist —
// it would work exactly the same way on a payoff with no formula at all
// (barrier options, Asian options, anything path-dependent).
double norm_cdf_d(double x) { return 0.5 * (1.0 + std::erf(x / std::sqrt(2.0))); }
double norm_pdf_d(double x) { return std::exp(-x * x / 2.0) / std::sqrt(2.0 * PI); }

int main() {
    double S0_val = 100.0;
    double K_val = 100.0;
    double r_val = 0.05;
    double sigma_val = 0.2;
    double T_val = 1.0;
    int num_paths = 50000;

    tape().clear();

    // These four are the only things we want derivatives with respect to.
    // Everything else (the random draws) are just plain numbers.
    Var S0(S0_val), r(r_val), sigma(sigma_val), T(T_val);

    std::mt19937 rng(42); // fixed seed so results are repeatable
    std::normal_distribution<double> normal(0.0, 1.0);

    Var total(0.0);

    for (int i = 0; i < num_paths; ++i) {
        double z = normal(rng); // one random draw per simulated path

        // simulate where the stock price ends up under this random path,
        // using the standard risk-neutral stock price model
        Var drift = (r - sigma * sigma * 0.5) * T;
        Var diffusion = sigma * sqrt(T) * z;
        Var S_T = S0 * exp(drift + diffusion);

        // payoff of a call option: what you'd get if the stock ends up at S_T
        Var payoff = (S_T.value > K_val) ? (S_T - K_val) : Var(0.0);

        // bring that future payoff back to today's value
        Var discounted = payoff * exp(-1.0 * r * T);

        total = total + discounted;
    }

    Var price = total / double(num_paths);

    // one backward pass through the ENTIRE simulation (all 50,000 paths)
    // gives us Delta, Vega, Rho, Theta -- despite there being no formula
    // for any of them here. This is the actual point of the whole project.
    std::vector<double> adj = tape().backward(price.index);

    double delta_mc = adj[S0.index];
    double vega_mc  = adj[sigma.index];
    double rho_mc   = adj[r.index];
    double theta_mc = -adj[T.index];

    // Black-Scholes formulas, purely so we can check the Monte Carlo
    // numbers landed in the right place (they'll be close, not exact,
    // since Monte Carlo has statistical noise; a formula has none).
    double d1 = (std::log(S0_val / K_val) + (r_val + sigma_val * sigma_val / 2.0) * T_val)
                / (sigma_val * std::sqrt(T_val));
    double d2 = d1 - sigma_val * std::sqrt(T_val);
    double bs_price = S0_val * norm_cdf_d(d1) - K_val * std::exp(-r_val * T_val) * norm_cdf_d(d2);
    double bs_delta = norm_cdf_d(d1);
    double bs_vega  = S0_val * norm_pdf_d(d1) * std::sqrt(T_val);
    double bs_rho   = K_val * T_val * std::exp(-r_val * T_val) * norm_cdf_d(d2);
    double bs_theta = -(S0_val * norm_pdf_d(d1) * sigma_val) / (2.0 * std::sqrt(T_val))
                      - r_val * K_val * std::exp(-r_val * T_val) * norm_cdf_d(d2);

    std::cout << "Monte Carlo price = " << price.value
              << "   (Black-Scholes = " << bs_price << ")\n\n";

    std::cout << "Delta (MC via AD) = " << delta_mc << "   (Black-Scholes) = " << bs_delta << "\n";
    std::cout << "Vega  (MC via AD) = " << vega_mc  << "   (Black-Scholes) = " << bs_vega  << "\n";
    std::cout << "Rho   (MC via AD) = " << rho_mc   << "   (Black-Scholes) = " << bs_rho   << "\n";
    std::cout << "Theta (MC via AD) = " << theta_mc << "   (Black-Scholes) = " << bs_theta << "\n";

    return 0;
}