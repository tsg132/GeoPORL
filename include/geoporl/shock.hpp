#pragma once

#include <array>

#include <random>

#include "state.hpp"

enum class ShockType {
    None = 0,
    Disaster,
    Coup,
    Sanction,
    Techbook,
    Pandemic,
    Escalation
};

// Shock stored for replay/logging
struct Shock {
    ShockType type;
    float severity;                  // [0,1]
    std::array<float, 10> embed;     // future LLM semantic vector
};

namespace geoporl {

// Apply stored shock in State to modify State
void apply_shock_effects(State& s);


// Sample a shock type & severity based on current risk
Shock sample_shock(const State& s, std::mt19937_64& rng);

} // namespace geoporl