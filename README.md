# mini-autodiff-cpp

A small automatic differentiation engine written from scratch in C++.

## What it does

Given a function, it computes both the function's value and its exact
derivative at the same time, in a single pass. No manual re-running of the
function with slightly nudged inputs, and no approximation.

This is done using a `Dual` number: a value that carries its derivative
alongside it. Every math operation (`+`, `-`, `*`, `/`, `sin`, `cos`, `exp`,
`log`, `sqrt`) is overloaded to update both the value and the derivative
using the standard calculus rules (product rule, chain rule, etc).

## Example

\`\`\`cpp
Dual x(2.0, 1.0);       // x = 2, seeded so we get df/dx
Dual result = x * x + sin(x);

// result.value -> f(2)
// result.deriv -> f'(2), computed automatically
\`\`\`

## Build and run

\`\`\`bash
g++ -std=c++17 -I include src/main.cpp -o demo
./demo
\`\`\`

## Status

This is forward-mode automatic differentiation: good for functions with
one input. Next step is adjoint (reverse) mode, which is what's needed for
functions with many inputs and one output, like option pricing Greeks.

## Why

This is a simplified version of the idea behind libraries like
[XAD](https://github.com/auto-differentiation/xad), used in quantitative
finance to compute risk sensitivities efficiently.