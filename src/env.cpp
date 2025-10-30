#include "geoporl/env.hpp"
#include "geoporl/dynamics.hpp"
#include "geoporl/shock.hpp"
#include <algorithm>

namespace {
inline double clamp01(double x){ return std::clamp(x,0.0,1.0); }

// Initialize a strong-America baseline state
State strong_america_baseline(){
    State s{};

    // Economy
    s.gdp_pc=0.85; s.unemployment=0.18; s.inflation=0.10; s.debt_to_gdp=0.86;
    s.trade_openness=0.60; s.foreign_direct_investment=0.55; s.economic_complexity=0.80;

    // Social
    s.population=0.85; s.education=0.70; s.healthcare=0.75; s.life_expectancy=0.75;
    s.inequality=0.50; s.urbanization=0.82; s.cohesion=0.60;

    // Governance
    s.democracy=0.75; s.civil_liberties=0.80; s.corruption=0.30;
    s.state_capacity=0.65; s.polarization=0.55; s.rule_of_law=0.70;

    // Security
    s.military_spend=0.95; s.global_force_projection=0.95; s.nuclear_deterrence=1.0;
    s.cyber_capability=0.90; s.internal_conflict_risk=0.25; s.external_threat=0.35; s.refugee_pressure=0.10;

    // Diplomacy
    s.alliance_value=0.95; s.soft_power=0.85; s.diplo_balance=0.65; s.sanction_risk=0.10;
    s.geo_location_value=0.80; s.autonomy=0.95;

    // Environment
    s.resource_dependency=0.19; s.rare_earth=0.30; s.water_scarcity=0.10;
    s.food_security=0.90; s.renewables_share=0.37; s.disaster_vulnerability=0.45;

    // Innovation
    s.rnd_investment=0.75; s.tech_adoption=0.85; s.ai_readiness=0.90; s.brain_drain=0.25; s.patent_output=0.85;

    // Systemic stress + shock fields
    s.systemic_stress=0.10;
    s.shock_severity=0.0f; s.shock_type=0; s.shock_embed={};

    s.t=0;
    return s;
}
} // anon namespace

Env::Env(uint64_t seed) : rng(seed) {}

void Env::reset() {
    s_ = strong_america_baseline();
}

double Env::step(int action, double& reward, std::array<double, 3>& costs) {
    // 1) Apply policy action
    if (action >= 0 && action < static_cast<int>(Action::COUNT)) {
        geoporl::dynamics::apply_action(s_, static_cast<Action>(action));
    }

    // 2) Apply any active shock effects (shock should be set externally)
    geoporl::apply_shock_effects(s_);

    // 3) Endogenous dynamics
    geoporl::dynamics::update_economy(s_);
    geoporl::dynamics::update_social(s_);
    geoporl::dynamics::update_governance(s_);
    geoporl::dynamics::update_security(s_);
    geoporl::dynamics::update_environment(s_);
    geoporl::dynamics::update_innovation(s_);
    geoporl::dynamics::advance_time(s_);

    // 4) Simple reward: weighted combination of key metrics
    reward = 0.3 * s_.gdp_pc 
           + 0.2 * s_.cohesion 
           + 0.2 * (1.0 - s_.internal_conflict_risk)
           + 0.15 * s_.education
           + 0.15 * s_.democracy;

    // 5) Costs (constraint violations for safe RL)
    costs[0] = std::max(0.0, s_.internal_conflict_risk - 0.40);
    costs[1] = std::max(0.0, s_.debt_to_gdp - 0.95);
    costs[2] = std::max(0.0, s_.systemic_stress - 0.50);

    // 6) Episode termination (only terminate on systemic collapse, let caller control max steps)
    const bool done = (s_.systemic_stress > 0.85);
    return done ? 1.0 : 0.0;
}

const State& Env::state() const {
    return s_;
}
