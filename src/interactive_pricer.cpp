#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <random>
#include "black_scholes.hpp"

// Asks the user for a number, showing a default they can accept by
// just pressing Enter. This is what makes the program usable by
// someone who isn't reading the source code first.
double ask(const std::string& label, double default_value) {
    std::cout << label << " [" << default_value << "]: ";
    std::string line;
    std::getline(std::cin, line);
    if (line.empty()) return default_value;
    return std::stod(line);
}

int main() {
    std::cout << "=== Option Pricer (Black-Scholes, via automatic differentiation) ===\n";
    std::cout << "Press Enter to accept the default shown in brackets.\n\n";

    double S = ask("Stock price (S)", 100.0);
    double K = ask("Strike price (K)", 100.0);
    double r = ask("Risk-free rate (r, e.g. 0.05 for 5%)", 0.05);
    double sigma = ask("Volatility (sigma, e.g. 0.2 for 20%)", 0.20);
    double T = ask("Time to expiry in years (T)", 1.0);
    int num_paths = (int)ask("Monte Carlo paths (for the simulated pricer)", 50000);

    // --- Closed-form-checked AD pricing ---
    CallGreeksAD call = call_greeks_ad(S, K, r, sigma, T);
    PutGreeksAD put = put_greeks_ad(S, K, r, sigma, T);
    double gamma = call_gamma_ad(S, K, r, sigma, T);

    std::cout << "\n--- Results ---\n";
    std::cout << "Call price = " << call.price << "\n";
    std::cout << "Put price  = " << put.price << "\n\n";

    std::cout << "Call Greeks:\n";
    std::cout << "  Delta = " << call.delta << "\n";
    std::cout << "  Gamma = " << gamma << "\n";
    std::cout << "  Vega  = " << call.vega << "\n";
    std::cout << "  Rho   = " << call.rho << "\n";
    std::cout << "  Theta = " << call.theta << "\n\n";

    std::cout << "Put Greeks:\n";
    std::cout << "  Delta = " << put.delta << "\n";
    std::cout << "  Vega  = " << put.vega << "\n";
    std::cout << "  Rho   = " << put.rho << "\n";
    std::cout << "  Theta = " << put.theta << "\n";

    // --- Write CSV for the Greeks-vs-price chart, centered on the user's K ---
    std::ofstream greeks_file("greeks_data.csv");
    greeks_file << "K=" << K << "\n";
    greeks_file << "S,price,delta,gamma,vega,rho,theta\n";
    double S_min = K * 0.5;
    double S_max = K * 1.5;
    int steps = 100;
    for (int i = 0; i <= steps; ++i) {
        double s = S_min + (S_max - S_min) * i / steps;
        CallGreeksAD g = call_greeks_ad(s, K, r, sigma, T);
        double gm = call_gamma_ad(s, K, r, sigma, T);
        greeks_file << s << "," << g.price << "," << g.delta << ","
                    << gm << "," << g.vega << "," << g.rho << "," << g.theta << "\n";
    }
    greeks_file.close();

    // --- Monte Carlo, using the user's own inputs, at increasing path counts ---
    std::ofstream mc_file("mc_convergence.csv");
    mc_file << "bs_price=" << call.price << ",bs_delta=" << call.delta << "\n";
    mc_file << "num_paths,price,delta\n";

    int path_counts[] = {100, 250, 500, 1000, 2500, 5000, 10000, 25000, 50000, 100000};
    for (int n : path_counts) {
        if (n > num_paths) break;
        tape().clear();
        Var S0v(S), rv(r), sigmav(sigma), Tv(T);
        std::mt19937 rng(42);
        std::normal_distribution<double> normal(0.0, 1.0);
        Var total(0.0);
        for (int i = 0; i < n; ++i) {
            double z = normal(rng);
            Var drift = (rv - sigmav * sigmav * 0.5) * Tv;
            Var diffusion = sigmav * sqrt(Tv) * z;
            Var S_T = S0v * exp(drift + diffusion);
            Var payoff = (S_T.value > K) ? (S_T - K) : Var(0.0);
            Var discounted = payoff * exp(-1.0 * rv * Tv);
            total = total + discounted;
        }
        Var mc_price = total / double(n);
        std::vector<double> adj = tape().backward(mc_price.index);
        mc_file << n << "," << mc_price.value << "," << adj[S0v.index] << "\n";
    }
    mc_file.close();

    std::cout << "\nWrote greeks_data.csv and mc_convergence.csv\n";
    std::cout << "Run: python plot_greeks.py   and   python plot_mc_convergence.py\n";

    return 0;
}