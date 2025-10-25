#include "geoporl/dynamics.hpp"
#include "geoporl/constants.hpp"
#include <algorithm>

using namespace geoporl::constants;

namespace {

// clamp helper
inline double clamp01(double x) {
    return std::clamp(x, 0.0, 1.0);
}

// gentle saturating increment
inline void bump_up(double& x, double delta) {
    x = clamp01(x + delta);
}

// gentle saturating decrement
inline void bump_down(double& x, double delta) {
    x = clamp01(x - delta);
}

} // anonymous

namespace geoporl::dynamics {

// ---------- Endogenous evolution ----------

void update_economy(State& s) {
    s.gdp_pc = clamp01(
        s.gdp_pc + GDP_BASE_GROWTH
      - s.inflation * INFLATION_GROWTH_IMPACT
    );

    s.unemployment = clamp01(
        s.unemployment + (UNEMPLOYMENT_GDP_ANCHOR - s.gdp_pc) * UNEMPLOYMENT_ADJ_RATE
    );

    s.inequality = clamp01(
        s.inequality + (s.gdp_pc - INEQUALITY_GDP_THRESHOLD) * (-INEQUALITY_ADJ_RATE)
    );

    // Very light coupling examples (placeholders)
    // Better GDP modestly improves soft power & trade openness
    s.soft_power       = clamp01(s.soft_power * (0.995) + 0.005 * s.gdp_pc);
    s.trade_openness   = clamp01(s.trade_openness * (0.997) + 0.003 * s.economic_complexity);
}

void update_social(State& s) {
    // Cohesion decays with inequality and internal conflict; rises with welfare proxies
    const double pressure = 0.6 * s.inequality + 0.4 * s.internal_conflict_risk;
    const double uplift   = 0.5 * s.education + 0.5 * s.healthcare;
    s.cohesion = clamp01( s.cohesion + 0.004 * (uplift - pressure) );

    // Urbanization slow drift upwards (MVP)
    s.urbanization = clamp01(s.urbanization + 0.0005);
}

void update_governance(State& s) {
    // Rule of law and state capacity mildly respond to cohesion and corruption
    s.rule_of_law    = clamp01(s.rule_of_law + 0.002 * (s.cohesion - s.corruption));
    s.state_capacity = clamp01(s.state_capacity + 0.002 * (s.cohesion - 0.5));

    // Polarization rises with inequality, falls with cohesion
    s.polarization = clamp01(s.polarization + 0.003 * (s.inequality - s.cohesion));
}

void update_security(State& s) {
    // Conflict escalates with inequality; small stabilization from governance
    s.internal_conflict_risk = clamp01(
        s.internal_conflict_risk
      + (s.inequality - CONFLICT_INEQUALITY_THRESHOLD) * CONFLICT_ESCALATION_RATE
      - 0.002 * s.rule_of_law
    );

    // External threat lightly anti-correlated with alliances & deterrence
    s.external_threat = clamp01(
        s.external_threat * 0.998
      + 0.002 * (1.0 - 0.6 * s.alliance_value - 0.4 * s.nuclear_deterrence)
    );
}

void update_environment(State& s) {
    // Renewables gently reduce resource dependency over time
    s.resource_dependency = clamp01(
        s.resource_dependency - 0.002 * s.renewables_share
    );

    // Disaster vulnerability has a floor but declines slightly with state capacity
    s.disaster_vulnerability = clamp01(
        s.disaster_vulnerability - 0.0015 * s.state_capacity
    );
}

void update_innovation(State& s) {
    // Brain drain falls as R&D rises (from constants)
    s.brain_drain = clamp01(
        s.brain_drain + (BRAIN_DRAIN_ANCHOR - s.rnd_investment) * BRAIN_DRAIN_ADJ_RATE
    );

    // Innovation feeds patent output and tech adoption
    s.patent_output = clamp01(s.patent_output * 0.99 + 0.01 * s.rnd_investment);
    s.tech_adoption = clamp01(s.tech_adoption * 0.995 + 0.005 * s.education);
}

// ---------- Actions (policy levers) ----------

void apply_action(State& s, Action a) {
    switch (a) {
    case Action::IncreaseDefenseSpending:
        bump_up(s.military_spend, 0.02);
        bump_up(s.debt_to_gdp, 0.01);
        bump_down(s.soft_power, 0.005);
        break;

    case Action::IncreaseEducationFunding:
        bump_up(s.education, 0.015);
        bump_up(s.debt_to_gdp, 0.005);
        bump_up(s.tech_adoption, 0.01);
        break;

    case Action::LowerTaxes:
        bump_up(s.gdp_pc, 0.01);
        bump_up(s.debt_to_gdp, 0.01);
        bump_up(s.inequality, 0.01);
        break;

    case Action::RaiseTaxes:
        bump_down(s.debt_to_gdp, 0.01);
        bump_down(s.inequality, 0.005);
        bump_down(s.gdp_pc, 0.01);
        break;

    case Action::InvestInInfrastructure:
        bump_up(s.state_capacity, 0.02);
        bump_up(s.gdp_pc, 0.005);
        bump_up(s.debt_to_gdp, 0.01);
        break;

    case Action::IncreaseWelfare:
        bump_up(s.cohesion, 0.015);
        bump_down(s.inequality, 0.01);
        bump_up(s.debt_to_gdp, 0.01);
        break;

    case Action::InvestInRenewables:
        bump_up(s.renewables_share, 0.02);
        bump_down(s.resource_dependency, 0.01);
        bump_up(s.debt_to_gdp, 0.01);
        break;

    case Action::ImproveCyberSecurity:
        bump_up(s.cyber_capability, 0.02);
        bump_down(s.rnd_investment, 0.005);
        break;

    case Action::StrengthenAlliances:
        bump_up(s.alliance_value, 0.02);
        bump_down(s.autonomy, 0.01);
        break;

    case Action::LimitImmigration:
        bump_up(s.polarization, 0.015);
        bump_up(s.brain_drain, 0.005);
        break;

    case Action::OpenImmigration:
        bump_down(s.polarization, 0.015);
        bump_down(s.brain_drain, 0.005);
        bump_up(s.population, 0.01);
        break;

    default:
        break;
    }
}

// ---------- Time progression ----------

void advance_time(State& s) {
    s.t += 1;
}

} // namespace geoporl::dynamics