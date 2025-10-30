#include "geoporl/shock.hpp"
#include <algorithm>
#include <cmath>
#include <random>

namespace {

inline double clamp01(double x) {
    return std::clamp(x, 0.0, 1.0);
}

// Maps state variables to risk contribution in [0,1]
double risk_factor(const State& s, ShockType type) {
    switch (type) {
        case ShockType::Disaster:
            return s.disaster_vulnerability;
        case ShockType::Coup:
            return 0.5*(1.0 - s.democracy) + 0.5*s.polarization;
        case ShockType::Sanction:
            return s.sanction_risk;
        case ShockType::Techboom:
            return s.rnd_investment * s.tech_adoption;
        case ShockType::Pandemic:
            return (1.0 - s.healthcare);
        case ShockType::Escalation:
            return s.external_threat;
        default:
            return 0.0;
    }
}

// Helper: Beta(α,β) via two Gammas
inline float sample_beta(double a, double b, std::mt19937_64& rng) {
    std::gamma_distribution<double> ga(a, 1.0);
    std::gamma_distribution<double> gb(b, 1.0);
    double x = ga(rng);
    double y = gb(rng);
    return float(x / (x + y));
}

} // anonymous namespace

namespace geoporl {

void init_shock_params(ShockParams& p) {
    for (int i = 0; i < 6; i++) {
        p.lambda[i] = 0.02;  // lower base event rate
        p.alpha[i]  = 2.0;   // mild shocks commonly
        p.beta[i]   = 6.0;   // but large shocks still possible
    }
}

// Weighted sampling based on risk
Shock sample_shock(const State& s,
                   const ShockParams& params,
                   std::mt19937_64& rng)
{
    std::uniform_real_distribution<double> U(0.0, 1.0);

    double P[6];
    double p_total = 0.0;

    for (int k = 0; k < 6; k++) {
        double rf = clamp01(
            risk_factor(s, static_cast<ShockType>(k+1))
        );
        P[k] = params.lambda[k] * rf;
        p_total += P[k];
    }

    if (U(rng) > p_total)
        return {ShockType::None, 0.0f, {}};

    // Weighted selection
    double r = U(rng) * p_total;
    for (int k = 0; k < 6; k++) {
        if (r < P[k]) {
            ShockType type = static_cast<ShockType>(k+1);
            float sev = sample_beta(params.alpha[k], params.beta[k], rng);
            return {type, sev, {}};
        }
        r -= P[k];
    }

    return {ShockType::None, 0.0f, {}};
}

// Minimal — updated as needed later
void apply_shock_effects(State& s) {
    if (s.shock_type == 0 || s.shock_severity <= 0.0f)
        return;

    float sev = s.shock_severity;

    switch (static_cast<ShockType>(s.shock_type)) {
    case ShockType::Disaster:
        s.gdp_pc                *= (1.0 - 0.02 * sev);
        s.cohesion              *= (1.0 - 0.03 * sev);
        s.food_security         *= (1.0 - 0.03 * sev);
        break;

    case ShockType::Coup:
        s.democracy             *= (1.0 - 0.04 * sev);
        s.state_capacity        *= (1.0 - 0.03 * sev);
        break;

    case ShockType::Sanction:
        s.trade_openness        *= (1.0 - 0.03 * sev);
        s.gdp_pc                *= (1.0 - 0.02 * sev);
        break;

    case ShockType::Techboom:
        s.rnd_investment        *= (1.0 + 0.02 * sev);
        s.tech_adoption         *= (1.0 + 0.01 * sev);
        break;

    case ShockType::Pandemic:
        s.healthcare            *= (1.0 - 0.03 * sev);
        s.life_expectancy       *= (1.0 - 0.03 * sev);
        break;

    case ShockType::Escalation:
        s.military_spend        *= (1.0 + 0.03 * sev);
        s.external_threat       *= (1.0 + 0.03 * sev);
        break;

    default: break;
    }

    // Decay shock next step
    s.shock_severity = std::max(0.0f, s.shock_severity - 0.05f);
}

} // namespace geoporl