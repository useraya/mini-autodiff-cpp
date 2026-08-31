#include <iostream>
#include "dual.hpp"

// The function we're testing: f(x) = x^2 + sin(x)
// By hand, its derivative is: f'(x) = 2x + cos(x)
// We use this because we already know the right answer,
// so we can check that our Dual type is actually correct.
Dual f(const Dual& x) {
    return x * x + sin(x);
}

int main() {
    double x_value = 2.0;

    // seed = 1.0 means "we're asking for the derivative with respect to x itself"
    Dual x(x_value, 1.0);

    Dual result = f(x);

    double expected_derivative = 2 * x_value + std::cos(x_value);

    std::cout << "f(x)        = " << result.value << "\n";
    std::cout << "f'(x) (AD)  = " << result.deriv << "\n";
    std::cout << "f'(x) (hand)= " << expected_derivative << "\n";

    return 0;
}