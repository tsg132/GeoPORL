#pragma once

namespace geoporl {

namespace constants
{

    static constexpr double GDP_BASE_GROWTH = 0.002;
    static constexpr double INFLATION_GROWTH_IMPACT = 0.0005;

    static constexpr double UNEMPLOYMENT_GDP_ANCHOR = 0.30;
    static constexpr double UNEMPLOYMENT_ADJ_RATE = 0.005;

    // === Inequality Dynamics ===
    static constexpr double INEQUALITY_GDP_THRESHOLD = 0.50;
    static constexpr double INEQUALITY_ADJ_RATE = 0.001;

    // === Conflict Dynamics ===
    static constexpr double CONFLICT_INEQUALITY_THRESHOLD = 0.45;
    static constexpr double CONFLICT_ESCALATION_RATE = 0.004;

    // === Innovation Dynamics ===
    static constexpr double BRAIN_DRAIN_ANCHOR = 0.60;
    static constexpr double BRAIN_DRAIN_ADJ_RATE = 0.002;

    // === Episode length ===
    static constexpr int MAX_TIMESTEPS = 200;

} // namespace constants


} //
