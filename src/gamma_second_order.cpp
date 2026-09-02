#include <iostream>
#include "var2.hpp"
#include "black_scholes2.hpp"
#include "dual.hpp"
#include "black_scholes_formulas.hpp"

using namespace ad2;

int main() {
    double S_val = 100.0, K_val = 100.0, r_val = 0.05, sigma_val = 0.2, T_val = 1.0;

    tape<Dual>().clear();

    // Here's the actual trick: instead of feeding in plain doubles, we feed
    // in Dual numbers. S is seeded with derivative 1.0 -- meaning "this is
    // the direction we're differentiating in". Everything else is seeded
    // with derivative 0.0, since we're not probing sensitivity to them
    // in this outer pass.
    Var<Dual> S(Dual(S_val, 1.0));
    Var<Dual> K(Dual(K_val, 0.0));
    Var<Dual> r(Dual(r_val, 0.0));
    Var<Dual> sigma(Dual(sigma_val, 0.0));
    Var<Dual> T(Dual(T_val, 0.0));

    Var<Dual> price = call_price(S, K, r, sigma, T);

    // Reverse-mode backward pass, same as always -- except every adjoint
    // that comes out is a Dual, not a plain number.
    std::vector<Dual> adj = tape<Dual>().backward(price.index);

    Dual delta_and_gamma = adj[S.index];

    // The .value here is the ordinary Delta (dPrice/dS), exactly like before.
    // The .deriv here is new: it's how Delta itself changes as S changes,
    // which is exactly Gamma -- computed exactly, not approximated by
    // nudging numbers and finite-differencing.
    double delta_ad = delta_and_gamma.value;
    double gamma_ad = delta_and_gamma.deriv;

    CallGreeksFormula formula = call_greeks_formula(S_val, K_val, r_val, sigma_val, T_val);

    std::cout << "Delta: AD (forward-over-reverse) = " << delta_ad
              << "   formula = " << formula.delta << "\n";
    std::cout << "Gamma: AD (forward-over-reverse) = " << gamma_ad
              << "   formula = " << formula.gamma << "\n";

    return 0;
}