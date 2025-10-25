#pragma once

#include "state.hpp"

#include <random>

#include <array>

class Env {

public:

    Env(uint64_t seed = 0); // Constructor

    void reset();

    double step(int action, double& reward, std::array<double, 3>& costs);

    const State& state() const;

private:

    State s_;

    std::mt19937_64 rng;


};
