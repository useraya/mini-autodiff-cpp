# mini-autodiff-cpp

A small automatic differentiation (AD) engine built from scratch in C++,
applied to option pricing.

## What problem this solves

In quantitative finance, banks need to know not just the price of a
financial instrument, but how sensitive that price is to each input
(stock price, volatility, interest rate, time). These sensitivities are
called Greeks (Delta, Vega, Rho, Theta, Gamma).

The usual way to get them is to slightly nudge one input and re-run the
whole pricing calculation to see how much the answer moved. This is slow
(one extra full run per input) and only approximate.

Automatic differentiation instead gets exact derivatives directly from the
calculation itself, in one pass, no matter how many inputs there are or
how complicated the calculation is. This project implements that technique
from first principles, then proves it works by applying it to real pricing
problems.

## What's included

**Forward-mode AD** (`include/dual.hpp`)
A `Dual` number that carries a value and its derivative together. Good for
functions with a single input.

**Reverse-mode / adjoint AD** (`include/tape.hpp`, `include/var.hpp`)
A `Var` number that records every operation onto a shared tape as the
calculation runs. Walking that tape backwards afterward gives the exact
derivative with respect to every input at once, computed in a single pass.
This is the technique that matters when there are many inputs and few
outputs, like Greeks.

**Black-Scholes Greeks** (`src/black_scholes_greeks.cpp`)
Call and put pricing built with `Var`, giving Delta, Vega, Rho, Theta, and
Gamma. Since Black-Scholes has known closed-form Greek formulas, this file
is used to check the AD engine's answers are exactly correct.

**Monte Carlo pricer with Greeks** (`src/monte_carlo_greeks.cpp`)
The real point of the project. A European call is priced by simulating
50,000 random stock price paths — there is no formula for a Greek here,
only a simulation. A single backward pass through the entire simulation
still produces Delta, Vega, Rho, and Theta, matching Black-Scholes within
the expected statistical noise. This is what real derivatives desks
actually need AD for: exotic and path-dependent options have no closed-form
Greeks at all, and this same method applies to them unchanged — only the
payoff function would need to change.

## Why Black-Scholes shows up twice

Black-Scholes is used only as a **validation case**, because its Greeks are
already known, so it's possible to check the AD engine got the right
answer. It is not the actual use case. The Monte Carlo file is the one that
demonstrates the real value: getting exact-in-expectation sensitivities out
of a calculation that has no formula to check against.

## Build and run

\`\`\`bash
# forward mode demo
g++ -std=c++17 -I include src/main.cpp -o demo
./demo

# Black-Scholes Greeks (calls, puts, Gamma)
g++ -std=c++17 -I include src/black_scholes_greeks.cpp -o greeks
./greeks

# Monte Carlo Greeks with no closed-form formula
g++ -std=c++17 -O2 -I include src/monte_carlo_greeks.cpp -o mc
./mc
\`\`\`

## Project structure

\`\`\`
mini-autodiff-cpp/
├── include/
│   ├── dual.hpp    # forward-mode AD
│   ├── tape.hpp    # records operations for reverse-mode AD
│   └── var.hpp     # reverse-mode AD number type, built on tape.hpp
├── src/
│   ├── main.cpp                  # forward-mode demo
│   ├── black_scholes_greeks.cpp  # reverse-mode AD, validated against formulas
│   └── monte_carlo_greeks.cpp    # reverse-mode AD through a simulation
└── README.md
\`\`\`

## Inspiration

This is a simplified version of the idea behind
[XAD](https://github.com/auto-differentiation/xad), a production automatic
differentiation library used in quantitative finance to compute risk
sensitivities on real pricing models.

## Possible next steps

- Second-order AD (true Gamma, not the finite-difference approximation used here)
- A path-dependent payoff with genuinely no closed-form Greeks (e.g. an Asian option)
- Python bindings so the engine can be called from Python pricing code