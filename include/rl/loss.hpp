#pragma once
#include <cmath>

namespace geoporl::rl {

// --- Base REINFORCE loss ---
inline float policy_gradient_loss(float log_prob, float advantage) {
    // L = - advantage * log_prob
    return -advantage * log_prob;
}

// --- Optional entropy bonus (for exploration) ---
inline float entropy_loss(float prob) {
    // Entropy = -sum(p * log(p))
    if (prob <= 0.0f) return 0.0f;
    return -prob * std::log(prob);
}

// --- Optional combined loss ---
inline float combined_loss(float log_prob, float advantage, float entropy_coeff, float prob) {
    return policy_gradient_loss(log_prob, advantage)
         - entropy_coeff * entropy_loss(prob);
}

} // namespace geoporl::rl