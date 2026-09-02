#pragma once
#include <vector>

// Same idea as tape.hpp, but generic: T can be a plain double (ordinary
// reverse-mode AD) or a Dual number (forward-mode AD from dual.hpp).
// When T is Dual, every "weight" recorded on the tape is itself a value
// that carries its own derivative -- which is exactly how forward-over-
// reverse mode works, and how we get an exact Gamma.
namespace ad2 {

template <typename T>
struct Node {
    int parent1;
    int parent2;
    T weight1;
    T weight2;
};

template <typename T>
class Tape {
public:
    std::vector<Node<T>> nodes;

    int push_leaf() {
        nodes.push_back({-1, -1, T(0.0), T(0.0)});
        return (int)nodes.size() - 1;
    }

    int push_unary(int parent, T weight) {
        nodes.push_back({parent, -1, weight, T(0.0)});
        return (int)nodes.size() - 1;
    }

    int push_binary(int parent1, T weight1, int parent2, T weight2) {
        nodes.push_back({parent1, parent2, weight1, weight2});
        return (int)nodes.size() - 1;
    }

    std::vector<T> backward(int output_index) {
        std::vector<T> adjoints(nodes.size(), T(0.0));
        adjoints[output_index] = T(1.0);

        for (int i = (int)nodes.size() - 1; i >= 0; --i) {
            const Node<T>& n = nodes[i];
            if (n.parent1 != -1) adjoints[n.parent1] = adjoints[n.parent1] + adjoints[i] * n.weight1;
            if (n.parent2 != -1) adjoints[n.parent2] = adjoints[n.parent2] + adjoints[i] * n.weight2;
        }
        return adjoints;
    }

    void clear() {
        nodes.clear();
    }
};

// One shared tape per type T. tape<double>() and tape<Dual>() are
// separate tapes, each with their own static storage.
template <typename T>
inline Tape<T>& tape() {
    static Tape<T> t;
    return t;
}

} // namespace ad2