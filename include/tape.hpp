#pragma once
#include <vector>

// Each Node is one recorded operation. It stores which earlier values
// it was built from (parent1, parent2) and how sensitive the result
// is to each of them (weight1, weight2). -1 means "no parent here".
struct Node {
    int parent1;
    int parent2;
    double weight1;
    double weight2;
};

// The Tape is the full history of a calculation, recorded step by step
// as it happens. Once the calculation is done, we walk the tape backwards
// to figure out how much the final result depends on every earlier value.
class Tape {
public:
    std::vector<Node> nodes;

    int push_leaf() {
        nodes.push_back({-1, -1, 0.0, 0.0});
        return (int)nodes.size() - 1;
    }

    int push_unary(int parent, double weight) {
        nodes.push_back({parent, -1, weight, 0.0});
        return (int)nodes.size() - 1;
    }

    int push_binary(int parent1, double weight1, int parent2, double weight2) {
        nodes.push_back({parent1, parent2, weight1, weight2});
        return (int)nodes.size() - 1;
    }

    // Walks the tape backwards from the output, spreading out
    // "how much did this affect the final answer" to every input.
    std::vector<double> backward(int output_index) {
        std::vector<double> adjoints(nodes.size(), 0.0);
        adjoints[output_index] = 1.0;

        for (int i = (int)nodes.size() - 1; i >= 0; --i) {
            double a = adjoints[i];
            if (a == 0.0) continue;
            const Node& n = nodes[i];
            if (n.parent1 != -1) adjoints[n.parent1] += a * n.weight1;
            if (n.parent2 != -1) adjoints[n.parent2] += a * n.weight2;
        }
        return adjoints;
    }

    void clear() {
        nodes.clear();
    }
};

// One shared tape for the whole program. Simple approach, fine for
// a single calculation at a time.
inline Tape& tape() {
    static Tape t;
    return t;
}