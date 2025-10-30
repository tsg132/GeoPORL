// #include "geoporl/metrics.hpp"

// #include <algorithm>
// #include <cmath>
// #include <array>

// namespace
// {

// inline double clamp01(double x) { // ask about inline
    
//     return std::clamp(x, 0.0, 1.0);

// }

// inline double log1p_scaled(double x, double k) {

//     return std::log1p(k * std::clamp(x, 0.0, 1.0)) / std::log1p(k);

// }

// } // namespace name


// namespace geoporl::metrics {

// double health_index(const State& s) {

//     return clamp01(0.6 * log1p_scaled(s.healthcare, 4.0) +
//                     0.4 * log1p_scaled(s.life_expectancy, 4.0));

// }

// double education_index(const State& s) {

//     return clamp01(log1p_scaled(s.education, 4.0));

// }

// double welfare(const State& s) {

//     const double g = log1p_scaled(s.gdp_pc, 4.0);
//     const double e = education_index(s);
//     const double h = health_index(s);

//     const double coh = s.cohesion;

//     return clamp01(0.4 * g + 0.3 * e + 0.2 * h + 0.1 * coh);


// }

// double corruption_risk(const State& s) {
//     // corruption already in [0,1], higher is worse
//     return clamp01(s.corruption);
// }

// double political_stability(const State& s) {
//     // inverse of polarization & internal conflict risk
//     const double pol = s.polarization;
//     const double ic  = s.internal_conflict_risk;
//     return clamp01(1.0 - 0.6 * ic - 0.4 * pol);
// }

// double governance(const State& s) {
//     // Quality of institutions + liberties + rule of law + execution capacity
//     const double base = 0.35 * s.democracy +
//                         0.25 * s.civil_liberties +
//                         0.20 * s.rule_of_law +
//                         0.20 * s.state_capacity;
//     // Stability modulates effectiveness (soft gating)
//     const double stab = political_stability(s);
//     return clamp01(0.8 * base + 0.2 * stab);
// }

// // ----- Power & Competitiveness -----

// double innovation(const State& s) {
//     // Linear for now (clear signal to dynamics)
//     return clamp01(0.5 * s.rnd_investment +
//                    0.3 * s.tech_adoption +
//                    0.2 * s.patent_output);
// }

// double competitiveness(const State& s) {
//     // Trade openness + economic complexity + soft power
//     return clamp01(0.4 * s.trade_openness +
//                    0.35 * s.economic_complexity +
//                    0.25 * s.soft_power);
// }

// double power(const State& s) {
//     // Multiplicative chain with square-root to temper extremes
//     const double proj = 0.7 * s.global_force_projection + 0.3 * s.cyber_capability;
//     const double raw  = std::max(0.0, s.military_spend) *
//                         std::max(0.0, proj) *
//                         std::max(0.0, s.alliance_value);
//     return clamp01(std::sqrt(std::min(raw, 1.0)));
// }

// // ----- Sustainability & Climate Risk -----

// double climate_risk(const State& s) {
//     // Water scarcity and disaster vulnerability are main drivers here
//     // resource_dependency can amplify climate risk if high
//     double risk = 0.45 * s.water_scarcity +
//                   0.40 * s.disaster_vulnerability +
//                   0.15 * s.resource_dependency;
//     return clamp01(risk);
// }

// double sustainability(const State& s) {
//     double sus = 0.4 * (1.0 - s.resource_dependency) +
//                  0.3 * s.renewables_share +
//                  0.2 * (1.0 - s.water_scarcity) +
//                  0.1 * (1.0 - s.disaster_vulnerability);
//     return clamp01(sus);
// }

// // ----- Risk / Cost signals -----

// double collapse_risk(const State& s) {
//     // Tipping behavior: take max of conflict & polarization, then square
//     const double r = std::max(s.internal_conflict_risk, s.polarization);
//     return clamp01(r * r);
// }

// double instability_cost(const State& s) {
//     // Combine collapse risk with corruption
//     const double c = corruption_risk(s);
//     const double k = collapse_risk(s);
//     return clamp01(0.7 * k + 0.3 * c);
// }

// // ----- Composite scoreboard (for dashboards) -----

// double composite_score(const State& s) {
//     // Not used for reward directly; nice for UI
//     const double w = welfare(s);
//     const double p = power(s);
//     const double sus = sustainability(s);
//     const double gov = governance(s);
//     const double inn = innovation(s);
//     const double cost = instability_cost(s);
//     // Simple average of positives minus a modest penalty
//     return clamp01((w + p + sus + gov + inn) / 5.0 - 0.3 * cost);
// }

// // ----- Vector features + weighted reward -----

// MetricVector compute(const State& s) {
//     MetricVector mv;
//     mv.welfare        = welfare(s);
//     mv.power          = power(s);
//     mv.sustainability = sustainability(s);
//     mv.governance     = governance(s);
//     mv.innovation     = innovation(s);
//     mv.instability_cost = instability_cost(s);
//     return mv;
// }

// double weighted_reward(const State& s, const std::array<double,6>& theta) {
//     const MetricVector m = compute(s);
//     // reward = θᵀ M(s); θ can be learned externally
//     return theta[0] * m.welfare
//          + theta[1] * m.power
//          + theta[2] * m.sustainability
//          + theta[3] * m.governance
//          + theta[4] * m.innovation
//          - theta[5] * m.instability_cost; // treat as penalty weight by convention
// }

// }

#include "geoporl/state.hpp"

namespace geoporl::metrics {

// Helper: smooth pin to a target band (e.g., inflation 2–6%)
static inline double band_penalty(double x, double lo, double hi, double k) {
    if (x < lo) {
        double d = lo - x; return -k * d * d;
    } else if (x > hi) {
        double d = x - hi; return -k * d * d;
    }
    return 0.0;
}

// Main reward: macro strength & stability (monthly scale).
// Returns roughly in [-3, +3] per month for typical ranges.
double compute_reward(const State& s) {
    // Positive drivers
    const double r_growth =
          1.00 * s.gdp_pc
        + 0.60 * s.cohesion
        + 0.20 * s.education
        + 0.20 * s.healthcare
        + 0.20 * s.soft_power;

    // Risks / costs
    const double r_costs =
        - 1.20 * s.internal_conflict_risk
        - 0.60 * s.debt_to_gdp
        - 0.50 * s.inflation
        - 0.40 * s.unemployment
        - 0.30 * s.corruption
        - 0.30 * s.external_threat;

    // Non-linear penalties
    // Debt cliff (heavier near 90%+)
    const double debt_cliff = (s.debt_to_gdp > 0.90)
        ? -2.5 * (s.debt_to_gdp - 0.90) * (s.debt_to_gdp - 0.90)
        : 0.0;

    // Inflation band: prefer 2%–6%
    const double infl_band = band_penalty(s.inflation, 0.02, 0.06, 12.0);

    // Conflict convexity (avoid “close to civil war” regimes)
    const double conflict_convex = -0.8 * s.internal_conflict_risk * s.internal_conflict_risk;

    // Systemic stress drag (small)
    const double stress_drag = -0.10 * s.systemic_stress;

    return r_growth + r_costs + debt_cliff + infl_band + conflict_convex + stress_drag;
}

} // namespace geoporl::metrics
