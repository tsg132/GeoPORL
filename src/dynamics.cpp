#include "geoporl/dynamics.hpp"
#include <algorithm>

namespace {
inline double clamp01(double x){ return std::clamp(x,0.0,1.0); }
inline void bump_up(double& x,double d){ x=clamp01(x+d); }
inline void bump_down(double& x,double d){ x=clamp01(x-d); }
}

namespace geoporl::dynamics {

void update_economy(State& s){

    /*

    GDP pc grows slowly based on state capacity, inflation harms GDP, unemployment ties inversely
    to GDP, economic health improves trade openness and soft power
    
    */

    // simple growth anchored by capacity; inflation drags
    const double cap = 1.0 + 0.25 * s.state_capacity;
    s.gdp_pc = std::clamp(s.gdp_pc + 0.003 * cap - 0.002 * s.inflation, 0.0, 1.0);

    s.unemployment = clamp01(s.unemployment + 0.02 * (0.5 - s.gdp_pc));
    s.inequality   = clamp01(s.inequality   - 0.005 * (s.cohesion - 0.5));
    s.trade_openness = clamp01(0.995 * s.trade_openness + 0.005 * s.economic_complexity);
    s.soft_power     = clamp01(0.998 * s.soft_power     + 0.002 * s.gdp_pc);
}


void update_social(State& s){

    /*

    Social cohesion reponds to educaion + healthcare (lift), inequality + internal conflict (pressure)
    
    */

    const double pressure = 0.6 * s.inequality + 0.4 * s.internal_conflict_risk;
    const double uplift   = 0.5 * s.education + 0.5 * s.healthcare;
    s.cohesion = clamp01(s.cohesion + 0.006 * (uplift - pressure));
    s.urbanization = clamp01(s.urbanization + 0.0004);
}

void update_governance(State& s){

    /*

    Social cohesion -> rule of law + state capacity

    inequality + conflict -> polarization
    
    */

    s.rule_of_law    = clamp01(s.rule_of_law + 0.002 * (s.cohesion - s.corruption));
    s.state_capacity = clamp01(s.state_capacity + 0.002 * (s.cohesion - 0.5));
    s.polarization   = clamp01(s.polarization + 0.003 * (s.inequality - s.cohesion)
                                            + 0.0015 * s.internal_conflict_risk);
}

void update_security(State& s){



    /*

    weak economy + poor cohesion = internal conflict, inequality is domininant driver, rule of law suppresses conflict.

    */

    
    const double econ_stress   = std::max(0.0, 0.6 - s.gdp_pc);
    const double social_stress = std::max(0.0, 0.4 - s.cohesion);
    s.internal_conflict_risk = clamp01(
        s.internal_conflict_risk
        + 0.008 * std::max(0.0, s.inequality - 0.5)
        + 0.003 * econ_stress
        + 0.003 * social_stress
        - 0.0015 * s.rule_of_law
    );

    s.external_threat = clamp01(
        0.998 * s.external_threat
      + 0.002 * (1.0 - 0.6 * s.alliance_value - 0.4 * s.nuclear_deterrence)
    );
}

void update_environment(State& s){
    s.resource_dependency     = clamp01(s.resource_dependency - 0.002 * s.renewables_share);
    s.disaster_vulnerability  = clamp01(s.disaster_vulnerability - 0.0012 * s.state_capacity);
}

void update_innovation(State& s){
    s.brain_drain   = clamp01(s.brain_drain + 0.01 * (0.5 - s.rnd_investment));
    s.patent_output = clamp01(0.99 * s.patent_output + 0.01 * s.rnd_investment);
    s.tech_adoption = clamp01(0.995 * s.tech_adoption + 0.005 * s.education);
}

// void apply_action(State& s, Action a){
//     switch(a){
//         case Action::IncreaseDefenseSpending:
//             bump_up(s.military_spend, 0.02); bump_up(s.debt_to_gdp, 0.01); break;
//         case Action::IncreaseEducationFunding:
//             bump_up(s.education, 0.015); bump_up(s.debt_to_gdp, 0.005); bump_up(s.tech_adoption, 0.01); break;
//         case Action::LowerTaxes:
//             bump_up(s.gdp_pc, 0.01); bump_up(s.debt_to_gdp, 0.01); bump_up(s.inequality, 0.01); break;
//         case Action::RaiseTaxes:
//             bump_down(s.gdp_pc, 0.01); bump_down(s.inequality, 0.005); bump_down(s.debt_to_gdp, 0.01); break;
//         case Action::InvestInInfrastructure:
//             bump_up(s.state_capacity, 0.02); bump_up(s.gdp_pc, 0.005); bump_up(s.debt_to_gdp, 0.01); break;
//         case Action::IncreaseWelfare:
//             bump_up(s.cohesion, 0.015); bump_down(s.inequality, 0.01); bump_up(s.debt_to_gdp, 0.01); break;
//         case Action::InvestInRenewables:
//             bump_up(s.renewables_share, 0.02); bump_down(s.resource_dependency, 0.01); bump_up(s.debt_to_gdp, 0.01); break;
//         case Action::ImproveCyberSecurity:
//             bump_up(s.cyber_capability, 0.02); bump_down(s.rnd_investment, 0.005); break;
//         case Action::StrengthenAlliances:
//             bump_up(s.alliance_value, 0.02); bump_down(s.autonomy, 0.01); break;
//         case Action::LimitImmigration:
//             bump_up(s.polarization, 0.015); bump_up(s.brain_drain, 0.005); break;
//         case Action::OpenImmigration:
//             bump_down(s.polarization, 0.015); bump_down(s.brain_drain, 0.005); bump_up(s.population, 0.01); break;
//         default: break;
//     }
// }

void apply_action(State& s, Action a) {
    switch (a) {
    case Action::IncreaseDefenseSpending:
        bump_up(s.military_spend,       0.0040);
        bump_up(s.debt_to_gdp,          0.0020);
        bump_down(s.soft_power,         0.0010);
        break;

    case Action::IncreaseEducationFunding:
        bump_up(s.education,            0.0030);
        bump_up(s.debt_to_gdp,          0.0010);
        bump_up(s.tech_adoption,        0.0020);
        break;

    case Action::LowerTaxes:
        bump_up(s.gdp_pc,               0.0030);
        bump_up(s.debt_to_gdp,          0.0020);
        bump_up(s.inequality,           0.0020);
        break;

    case Action::RaiseTaxes:
        bump_down(s.debt_to_gdp,        0.0030);
        bump_down(s.inequality,         0.0015);
        bump_down(s.gdp_pc,             0.0015);
        break;

    case Action::InvestInInfrastructure:
        bump_up(s.state_capacity,       0.0040);
        bump_up(s.gdp_pc,               0.0010);
        bump_up(s.debt_to_gdp,          0.0020);
        break;

    case Action::IncreaseWelfare:
        bump_up(s.cohesion,             0.0030);
        bump_down(s.inequality,         0.0020);
        bump_up(s.debt_to_gdp,          0.0020);
        break;

    case Action::InvestInRenewables:
        bump_up(s.renewables_share,     0.0040);
        bump_down(s.resource_dependency,0.0020);
        bump_up(s.debt_to_gdp,          0.0020);
        break;

    case Action::ImproveCyberSecurity:
        bump_up(s.cyber_capability,     0.0040);
        bump_down(s.rnd_investment,     0.0010);
        break;

    case Action::StrengthenAlliances:
        bump_up(s.alliance_value,       0.0040);
        bump_down(s.autonomy,           0.0020);
        break;

    case Action::LimitImmigration:
        bump_up(s.polarization,         0.0030);
        bump_up(s.brain_drain,          0.0010);
        break;

    case Action::OpenImmigration:
        bump_down(s.polarization,       0.0030);
        bump_down(s.brain_drain,        0.0010);
        bump_up(s.population,           0.0010);
        break;

    default:
        break;
    }
}

void advance_time(State& s){ s.t += 1; }

} // namespace