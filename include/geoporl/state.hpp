#pragma once

#include <array>

// The State struct stores everything about a country at a time step.
// All values are normalized to [0,1] for stability in RL.
struct State {

    // === Economics ===
    double gdp_pc;
    double unemployment;
    double inflation;
    double debt_to_gdp;
    double trade_openness;
    double foreign_direct_investment;
    double economic_complexity;

    // === Social ===
    double population;
    double education;
    double healthcare;
    double life_expectancy;
    double inequality;
    double urbanization;
    double cohesion;

    // === Governance & Domestic Politics ===
    double democracy;
    double civil_liberties;
    double corruption;
    double state_capacity;
    double polarization;
    double rule_of_law;

    // === Security & Military ===
    double military_spend;
    double global_force_projection;
    double nuclear_deterrence;
    double cyber_capability;
    double internal_conflict_risk;
    double external_threat;
    double refugee_pressure;

    // === Diplomacy & Alliances ===
    double alliance_value;
    double soft_power;
    double diplo_balance;       // relation with global blocs
    double sanction_risk;
    double geo_location_value;  // chokepoints, trade influence
    double autonomy;            // reliance on others

    // === Resources & Environment ===
    double resource_dependency;
    double rare_earth;
    double water_scarcity;
    double food_security;
    double renewables_share;
    double disaster_vulnerability;

    // === Innovation & Competitiveness ===
    double rnd_investment;
    double tech_adoption;
    double ai_readiness;
    double brain_drain;
    double patent_output;

    double systemic_stress;

    // === Shock Module (connected to LLM later) ===
    float shock_severity;          // [0,1]
    int   shock_type;              // categorical index
    std::array<float, 10> shock_embed;  // semantic vector

    int t; // timestep in episode
};