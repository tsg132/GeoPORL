#pragma once
#include "state.hpp"
#include "actions.hpp"

namespace geoporl::dynamics {

// === Endogenous evolution (economics, society, tech, etc.) ===
void update_economy(State& s);
void update_social(State& s);
void update_governance(State& s);
void update_security(State& s);
void update_environment(State& s);
void update_innovation(State& s);

// === Apply Action Effects (Policy levers from agent) ===
void apply_action(State& s, Action a);

// === Time progression (structural drift, temporal counters, etc.) ===
void advance_time(State& s);

} // namespace geoporl::dynamics