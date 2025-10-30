#include "geoporl/env.hpp"
#include "geoporl/actions.hpp"
#include "geoporl/shock.hpp"
#include "geoporl/dynamics.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <random>
#include <iomanip>

int main() {

    const int NUM_SIMULATIONS = 100;
    const int STEPS = 120;
    const uint64_t BASE_SEED = 42;

    std::cout << "Running " << NUM_SIMULATIONS << " GeoPORL simulations with " << STEPS << " timesteps each...\n";

    for (int sim = 0; sim < NUM_SIMULATIONS; sim++) {
        
        uint64_t seed = BASE_SEED + sim;
        std::mt19937_64 rng(seed);
        std::uniform_int_distribution<int> action_dist(1, static_cast<int>(Action::COUNT) - 1);

        Env env(seed);
        ShockParams shock_params;
        geoporl::init_shock_params(shock_params);

        env.reset();

        // Create CSV filename for this simulation
        std::ostringstream filename;
        filename << "simulation_" << std::setw(3) << std::setfill('0') << sim << ".csv";
        
        std::ofstream csvfile(filename.str());
        csvfile << "t,action,reward,cost0,cost1,cost2,"
                << "gdp_pc,cohesion,internal_conflict_risk,shock_severity,shock_type,"
                << "unemployment,inflation,debt_to_gdp,democracy,education,healthcare,"
                << "inequality,polarization,state_capacity,systemic_stress\n";

        for (int t = 0; t < STEPS; t++) {
            int action = action_dist(rng);

            Shock shock = geoporl::sample_shock(env.state(), shock_params, rng);
            
            State& s_mutable = const_cast<State&>(env.state());
            if (shock.type != ShockType::None) {
                s_mutable.shock_severity = shock.severity;
                s_mutable.shock_type = static_cast<int>(shock.type);
                s_mutable.shock_embed = shock.embed;
            }

            double reward = 0.0;
            std::array<double, 3> costs = {0.0, 0.0, 0.0};
            double done = env.step(action, reward, costs);

            const State& s = env.state();

            csvfile << s.t << "," << action << "," << reward << ","
                    << costs[0] << "," << costs[1] << "," << costs[2] << ","
                    << s.gdp_pc << "," << s.cohesion << "," << s.internal_conflict_risk << ","
                    << s.shock_severity << "," << s.shock_type << ","
                    << s.unemployment << "," << s.inflation << "," << s.debt_to_gdp << ","
                    << s.democracy << "," << s.education << "," << s.healthcare << ","
                    << s.inequality << "," << s.polarization << "," << s.state_capacity << ","
                    << s.systemic_stress << "\n";

            if (done > 0.5) {
                break;
            }
        }

        csvfile.close();

        if ((sim + 1) % 10 == 0) {
            std::cout << "Completed " << (sim + 1) << " / " << NUM_SIMULATIONS << " simulations\n";
        }
    }

    std::cout << "\nAll simulations complete. Results saved to simulation_*.csv\n";

    return 0;
}

