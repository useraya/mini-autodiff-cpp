#include <iostream>
#include <fstream>
#include "black_scholes.hpp"

// Sweeps the stock price across a range, computing price and all Greeks
// via AD at each point, and writes the results to a CSV file. This is
// what actually makes a useful chart: Greeks aren't interesting as a
// single number, they're interesting as a *curve* -- how Delta rises
// from 0 to 1 as an option goes from out-of-the-money to in-the-money,
// how Gamma peaks exactly at-the-money, etc.
int main() {
    double K_val = 100.0;
    double r_val = 0.05;
    double sigma_val = 0.2;
    double T_val = 1.0;

    std::ofstream file("greeks_data.csv");
    file << "S,price,delta,gamma,vega,rho,theta\n";

    for (double S_val = 50.0; S_val <= 150.0; S_val += 1.0) {
        CallGreeksAD g = call_greeks_ad(S_val, K_val, r_val, sigma_val, T_val);
        double gamma = call_gamma_ad(S_val, K_val, r_val, sigma_val, T_val);

        file << S_val << "," << g.price << "," << g.delta << ","
             << gamma << "," << g.vega << "," << g.rho << "," << g.theta << "\n";
    }

    file.close();
    std::cout << "Wrote greeks_data.csv\n";
    return 0;
}