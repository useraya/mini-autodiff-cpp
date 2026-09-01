#include <iostream>
#include <cmath>
#include <string>
#include "black_scholes.hpp"
#include "black_scholes_formulas.hpp"

int total_checks = 0;
int failed_checks = 0;

// Compares one AD value against one formula value, prints PASS/FAIL.
void check(const std::string& label, double ad_value, double formula_value, double tol = 1e-4) {
    total_checks++;
    double diff = std::fabs(ad_value - formula_value);
    bool pass = diff < tol;
    if (!pass) failed_checks++;

    std::cout << (pass ? "[PASS] " : "[FAIL] ") << label
              << "  AD=" << ad_value << "  formula=" << formula_value
              << "  diff=" << diff << "\n";
}

// Runs the full set of checks for one combination of inputs.
void run_case(const std::string& case_name, double S, double K, double r, double sigma, double T) {
    std::cout << "\n-- " << case_name
              << "  (S=" << S << " K=" << K << " r=" << r
              << " sigma=" << sigma << " T=" << T << ") --\n";

    CallGreeksAD ad = call_greeks_ad(S, K, r, sigma, T);
    CallGreeksFormula formula = call_greeks_formula(S, K, r, sigma, T);
    double gamma_ad = call_gamma_ad(S, K, r, sigma, T);

    check("Price", ad.price, formula.price);
    check("Delta", ad.delta, formula.delta);
    check("Vega",  ad.vega,  formula.vega);
    check("Rho",   ad.rho,   formula.rho);
    check("Theta", ad.theta, formula.theta);
    check("Gamma", gamma_ad, formula.gamma, 1e-3); // looser tolerance: finite-difference based
}

int main() {
    // A spread of realistic and edge-ish scenarios: at-the-money,
    // deep in/out of the money, high volatility, and short expiry.
    run_case("At-the-money",        100.0, 100.0, 0.05, 0.20, 1.0);
    run_case("Deep in-the-money",   150.0, 100.0, 0.05, 0.20, 1.0);
    run_case("Deep out-of-the-money", 60.0, 100.0, 0.05, 0.20, 1.0);
    run_case("High volatility",     100.0, 100.0, 0.05, 0.80, 1.0);
    run_case("Short expiry",        100.0, 100.0, 0.05, 0.20, 0.05);
    run_case("Long expiry",         100.0, 100.0, 0.05, 0.20, 5.0);
    run_case("High interest rate",  100.0, 100.0, 0.15, 0.20, 1.0);
    run_case("Low volatility",      100.0, 100.0, 0.05, 0.05, 1.0);

    std::cout << "\n============================\n";
    std::cout << total_checks - failed_checks << " / " << total_checks << " checks passed\n";
    std::cout << "============================\n";

    return failed_checks == 0 ? 0 : 1;
}