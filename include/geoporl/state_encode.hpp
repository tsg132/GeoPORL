#pragma once
#include "geoporl/state.hpp"
#include <vector>

// Convert State to flat float vector for policy input
inline std::vector<float> encode_state(const State& s) {
    std::vector<float> out;

    // Reserve roughly 70 features (for efficiency)
    out.reserve(70);

    auto pushd = [&](double v) { out.push_back(static_cast<float>(v)); };
    auto pushf = [&](float v) { out.push_back(v); };

    // === Economics ===
    pushd(s.gdp_pc);
    pushd(s.unemployment);
    pushd(s.inflation);
    pushd(s.debt_to_gdp);
    pushd(s.trade_openness);
    pushd(s.foreign_direct_investment);
    pushd(s.economic_complexity);

    // === Social ===
    pushd(s.population);
    pushd(s.education);
    pushd(s.healthcare);
    pushd(s.life_expectancy);
    pushd(s.inequality);
    pushd(s.urbanization);
    pushd(s.cohesion);

    // === Governance ===
    pushd(s.democracy);
    pushd(s.civil_liberties);
    pushd(s.corruption);
    pushd(s.state_capacity);
    pushd(s.polarization);
    pushd(s.rule_of_law);

    // === Security ===
    pushd(s.military_spend);
    pushd(s.global_force_projection);
    pushd(s.nuclear_deterrence);
    pushd(s.cyber_capability);
    pushd(s.internal_conflict_risk);
    pushd(s.external_threat);
    pushd(s.refugee_pressure);

    // === Diplomacy ===
    pushd(s.alliance_value);
    pushd(s.soft_power);
    pushd(s.diplo_balance);
    pushd(s.sanction_risk);
    pushd(s.geo_location_value);
    pushd(s.autonomy);

    // === Resources ===
    pushd(s.resource_dependency);
    pushd(s.rare_earth);
    pushd(s.water_scarcity);
    pushd(s.food_security);
    pushd(s.renewables_share);
    pushd(s.disaster_vulnerability);

    // === Innovation ===
    pushd(s.rnd_investment);
    pushd(s.tech_adoption);
    pushd(s.ai_readiness);
    pushd(s.brain_drain);
    pushd(s.patent_output);

    // === Other ===
    pushd(s.systemic_stress);
    pushf(s.shock_severity);
    out.push_back(static_cast<float>(s.shock_type));
    for (float v : s.shock_embed) pushf(v);

    pushf(static_cast<float>(s.t));

    return out;
}