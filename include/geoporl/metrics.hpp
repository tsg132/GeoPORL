#pragma once
#include "state.hpp"

namespace geoporl::metrics {

// Core human development
double welfare(const State& s);
double health_index(const State& s);
double education_index(const State& s);

// Governance
double governance(const State& s);
double corruption_risk(const State& s);
double political_stability(const State& s);

// Power & Competitiveness
double power(const State& s);
double innovation(const State& s);
double competitiveness(const State& s);

// Sustainability
double sustainability(const State& s);
double climate_risk(const State& s);

// Risk / Cost signal metrics
double collapse_risk(const State& s);
double instability_cost(const State& s);

// Global scoreboard
double composite_score(const State& s);

} // namespace geoporl::metrics

namespace geoporl::metrics {
struct MetricVector {
    double welfare;
    double power;
    double sustainability;
    double governance;
    double innovation;
    double instability_cost;
};

MetricVector compute(const State& s);

// reward = theta dot compute(s)
double weighted_reward(const State& s,
                       const std::array<double,6>& theta);
}