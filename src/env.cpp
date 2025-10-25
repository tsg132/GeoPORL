#include "geoporl/env.hpp"
#include "geoporl/constants.hpp"
#include "geoporl/dynamics.hpp"
#include "geoporl/metrics.hpp"
#include "geoporl/actions.hpp"
#include "geoporl/shock.hpp"

#include <algorithm>
#include <array>

using namespace geoporl::constants;

namespace {
inline double clamp01(double x) { return std::clamp(x, 0.0, 1.0); }
} // anonymous

Env::Env(uint64_t seed) : rng(seed) {
    reset();
}

void Env::reset() {
    s_ = State{};
    s_.t = 0;

    s_.systemic_stress = 0;

    // --- USA-like baseline (normalized) ---
    // Economics
    s_.gdp_pc = 0.90;
    s_.unemployment = 0.20;
    s_.inflation = 0.10;
    s_.debt_to_gdp = 0.85;
    s_.trade_openness = 0.60;
    s_.foreign_direct_investment = 0.55;
    s_.economic_complexity = 0.80;

    // Social
    s_.population = 0.85;
    s_.education = 0.70;
    s_.healthcare = 0.65;
    s_.life_expectancy = 0.75;
    s_.inequality = 0.50;
    s_.urbanization = 0.82;
    s_.cohesion = 0.60;

    // Governance
    s_.democracy = 0.75;
    s_.civil_liberties = 0.80;
    s_.corruption = 0.30;
    s_.state_capacity = 0.65;
    s_.polarization = 0.55;
    s_.rule_of_law = 0.70;

    // Security
    s_.military_spend = 0.95;
    s_.global_force_projection = 0.95;
    s_.nuclear_deterrence = 1.0;
    s_.cyber_capability = 0.90;
    s_.internal_conflict_risk = 0.25;
    s_.external_threat = 0.35;
    s_.refugee_pressure = 0.10;

    // Diplomacy
    s_.alliance_value = 0.95;
    s_.soft_power = 0.85;
    s_.diplo_balance = 0.65;
    s_.sanction_risk = 0.10;
    s_.geo_location_value = 0.80;
    s_.autonomy = 0.95;

    // Resources & Environment
    s_.resource_dependency = 0.20;
    s_.rare_earth = 0.30;
    s_.water_scarcity = 0.10;
    s_.food_security = 0.90;
    s_.renewables_share = 0.35;
    s_.disaster_vulnerability = 0.45;

    // Innovation
    s_.rnd_investment = 0.75;
    s_.tech_adoption = 0.85;
    s_.ai_readiness = 0.90;
    s_.brain_drain = 0.25;
    s_.patent_output = 0.85;

    // Shocks
    s_.shock_severity = 0.0f;
    s_.shock_type = 0;
    s_.shock_embed.fill(0.0f);
}

double Env::step(int action, double& reward, std::array<double, 3>& costs) {
    // 1) Apply policy lever
    if (action >= 0 && action < static_cast<int>(Action::ACTION_COUNT)) {
        geoporl::dynamics::apply_action(s_, static_cast<Action>(action));
    }

    // 2) Sample and apply shocks
    Shock shock = geoporl::sample_shock(s_, rng);
    if (shock.type != ShockType::None) {
        s_.shock_severity = shock.severity;
        s_.shock_type = static_cast<int>(shock.type);
        s_.shock_embed = shock.embed;
    }
    geoporl::apply_shock_effects(s_);

    // 3) Endogenous evolution
    geoporl::dynamics::update_economy(s_);
    geoporl::dynamics::update_social(s_);
    geoporl::dynamics::update_governance(s_);
    geoporl::dynamics::update_security(s_);
    geoporl::dynamics::update_environment(s_);
    geoporl::dynamics::update_innovation(s_);
    geoporl::dynamics::advance_time(s_);

    // 4) Reward (using learnable linear weights — for now, a baseline θ)
    // You can later expose theta via an Env setter/API or through Python bindings.
    const std::array<double,6> theta = {
        0.40, // welfare
        0.20, // power
        0.15, // sustainability
        0.15, // governance
        0.10, // innovation
        0.30  // instability penalty weight (subtracted in metrics::weighted_reward)
    };
    reward = geoporl::metrics::weighted_reward(s_, theta);

    // 5) Costs (constraints for Lagrangian PPO)
    costs[0] = std::max(0.0, s_.internal_conflict_risk - 0.30);
    costs[1] = std::max(0.0, s_.debt_to_gdp        - 0.90);
    costs[2] = 0.0; // reserved

    // 6) Done?
    const bool done = (s_.t >= MAX_TIMESTEPS);
    return done ? 1.0 : 0.0;
}

const State& Env::state() const {
    return s_;
}