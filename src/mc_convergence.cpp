#include <iostream>
#include <fstream>
#include <cmath>
#include <random>
#include "var.hpp"

// Runs the Monte Carlo AD pricer at a given number of paths, returns
// price and Delta. Used to show how the estimate settles down as more
// paths are simulated -- the real tradeoff in Monte Carlo methods:
// more paths = more accurate, but more compute time.
struct MCResult {
    double price;
    double delta;
};

MCResult run_monte_carlo(int num_paths, double S0_val, double K_val,
                          double r_val, double sigma_val, double T_val, unsigned seed) {
    tape().clear();
    Var S0(S0_val), r(r_val), sigma(sigma_val), T(T_val);

    std::mt19937 rng(seed);
    std::normal_distribution<double> normal(0.0, 1.0);

    Var total(0.0);
    for (int i = 0; i < num_paths; ++i) {
        double z = normal(rng);
        Var drift = (r - sigma * sigma * 0.5) * T;
        Var diffusion = sigma * sqrt(T) * z;
        Var S_T = S0 * exp(drift + diffusion);
        Var payoff = (S_T.value > K_val) ? (S_T - K_val) : Var(0.0);
        Var discounted = payoff * exp(-1.0 * r * T);
        total = total + discounted;
    }

    Var price = total / double(num_paths);
    std::vector<double> adj = tape().backward(price.index);

    return { price.value, adj[S0.index] };
}

int main() {
    double S0_val = 100.0, K_val = 100.0, r_val = 0.05, sigma_val = 0.2, T_val = 1.0;

    std::ofstream file("mc_convergence.csv");
    file << "num_paths,price,delta\n";

    int path_counts[] = {100, 250, 500, 1000, 2500, 5000, 10000, 25000, 50000, 100000, 200000};

    for (int n : path_counts) {
        MCResult r = run_monte_carlo(n, S0_val, K_val, r_val, sigma_val, T_val, 42);
        file << n << "," << r.price << "," << r.delta << "\n";
        std::cout << "paths=" << n << "  price=" << r.price << "  delta=" << r.delta << "\n";
    }

    file.close();
    std::cout << "Wrote mc_convergence.csv\n";
    return 0;
}