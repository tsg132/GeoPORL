#pragma once
#include "state.hpp"
#include "actions.hpp"

namespace geoporl::dynamics {

// deterministic endogenous updates (one tick)
void update_economy(State& s);
void update_social(State& s);
void update_governance(State& s);
void update_security(State& s);
void update_environment(State& s);
void update_innovation(State& s);

// apply one policy action
void apply_action(State& s, Action a);

// advance time counter
void advance_time(State& s);

} // namespace geoporl::dynamics