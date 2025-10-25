#include "geoporl/shock.hpp"
#include <algorithm>
#include <cmath>
#include <random>

namespace {

// clamp helper
inline double clamp01(double x) {
    return std::clamp(x, 0.0, 1.0);
}

// Gradual recovery
inline void decay_severity(float& sev) {
    sev = std::max(0.0f, sev - 0.05f);
}

// Scalar shock impact utility
inline void apply_scalar(double& target, float sev, double magnitude, bool positive) {
    if (positive)
        target = clamp01(target + sev * magnitude);
    else
        target = clamp01(target - sev * magnitude);
}

} // anonymous namespace

namespace geoporl {

// ===========================
// Shock: Effects on the State
// ===========================
void apply_shock_effects(State& s) {
    const float sev = clamp01(s.shock_severity);
    if (sev <= 0.001f || s.shock_type == 0)
        return;

    switch (static_cast<ShockType>(s.shock_type)) {

    case ShockType::Disaster:
        apply_scalar(s.gdp_pc, sev, 0.05, false);
        apply_scalar(s.cohesion, sev, 0.05, false);
        apply_scalar(s.disaster_vulnerability, sev, 0.03, true);
        apply_scalar(s.food_security, sev, 0.06, false);
        apply_scalar(s.water_scarcity, sev, 0.04, true);
        break;

    case ShockType::Coup:
        apply_scalar(s.democracy, sev, 0.08, false);
        apply_scalar(s.state_capacity, sev, 0.06, false);
        apply_scalar(s.polarization, sev, 0.05, true);
        apply_scalar(s.internal_conflict_risk, sev, 0.06, true);
        break;

    case ShockType::Sanction:
        apply_scalar(s.trade_openness, sev, 0.05, false);
        apply_scalar(s.gdp_pc, sev, 0.04, false);
        apply_scalar(s.foreign_direct_investment, sev, 0.07, false);
        apply_scalar(s.sanction_risk, sev, 0.05, true);
        break;

    case ShockType::Techbook:
        apply_scalar(s.rnd_investment, sev, 0.05, true);
        apply_scalar(s.tech_adoption, sev, 0.04, true);
        apply_scalar(s.patent_output, sev, 0.05, true);
        break;

    case ShockType::Pandemic:
        apply_scalar(s.healthcare, sev, 0.07, false);
        apply_scalar(s.life_expectancy, sev, 0.07, false);
        apply_scalar(s.education, sev, 0.04, false);
        apply_scalar(s.internal_conflict_risk, sev, 0.05, true);
        break;

    case ShockType::Escalation:
        apply_scalar(s.military_spend, sev, 0.06, true);
        apply_scalar(s.external_threat, sev, 0.06, true);
        apply_scalar(s.internal_conflict_risk, sev, 0.04, true);
        apply_scalar(s.gdp_pc, sev, 0.05, false);
        break;

    default:
        break;
    }

    decay_severity(s.shock_severity);
}

// ===========================
// Shock Sampling (Risk-based)
// ===========================
Shock sample_shock(const State& s, std::mt19937_64& rng) {
    std::uniform_real_distribution<double> u(0.0, 1.0);
    double roll = u(rng);

    // Probabilities proportional to risk signals
    double p_disaster    = s.disaster_vulnerability * 0.15;
    double p_coup        = s.polarization * 0.10;
    double p_sanction    = s.sanction_risk * 0.08;
    double p_techboom    = s.rnd_investment * s.tech_adoption * 0.05;
    double p_pandemic    = (1.0 - s.healthcare) * 0.12;
    double p_escalation  = s.external_threat * 0.10;

    double p_total = p_disaster + p_coup + p_sanction +
                     p_techboom + p_pandemic + p_escalation;

    if (roll > p_total)
        return {ShockType::None, 0.0f, {}};

    // Weighted selection
    double r = roll * p_total;

    if ((r -= p_disaster) <= 0)   return {ShockType::Disaster,   float(u(rng)), {}};
    if ((r -= p_coup) <= 0)       return {ShockType::Coup,       float(u(rng)), {}};
    if ((r -= p_sanction) <= 0)   return {ShockType::Sanction,   float(u(rng)), {}};
    if ((r -= p_techboom) <= 0)   return {ShockType::Techbook,   float(u(rng)), {}};
    if ((r -= p_pandemic) <= 0)   return {ShockType::Pandemic,   float(u(rng)), {}};

    return {ShockType::Escalation, float(u(rng)), {}};
}

} // namespace geoporl