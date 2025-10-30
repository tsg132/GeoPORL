#pragma once

#include <array>
#include <random>
#include "state.hpp"

enum class ShockType {
    None = 0,
    Disaster,
    Coup,
    Sanction,
    Techboom,
    Pandemic,
    Escalation
};

// Parameters defining Poisson rate + severity distribution
struct ShockParams {
    std::array<double, 6> lambda;  // shock intensity weighting
    std::array<double, 6> alpha;   // beta distribution α
    std::array<double, 6> beta;    // beta distribution β
};

// Shock stored for replay/logging
struct Shock {
    ShockType type;
    float severity;                       // [0,1]
    std::array<float, 10> embed {};       // for later LLM embeddings
};

namespace geoporl {

// Initialize λ,α,β from priors (later we learn from real data)
void init_shock_params(ShockParams&);

// Sample a shock and generate a severity
Shock sample_shock(const State&,
                   const ShockParams&,
                   std::mt19937_64&);

// Apply the stored shock in state to modify state
void apply_shock_effects(State&);

} // namespace geoporl