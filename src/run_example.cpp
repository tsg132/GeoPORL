#include "geoporl/env.hpp"

#include "geoporl/replay.hpp"
#include "geoporl/actions.hpp"

#include <iostream>
#include <random>

int main() {

    Env env(42);

    ReplayBuffer replay;

    const int EPISODE_ID = 0;

    const int STEPS = 200;

    std::mt19937_64 rng(42);

    std::uniform_int_distribution<int> dist(0, static_cast<int>(Action::ACTION_COUNT) - 1);

    env.reset();

    for (int t = 0; t < STEPS; t++) {

        const int action = dist(rng);

        double reward = 0.0;

        std::array<double, 3> costs{0.0, 0.0, 0.0};

        const double done = env.step(action, reward, costs);

        const State& s = env.state();

        ReplayStep step;

        step.episode_id = EPISODE_ID;

        step.t = t;

        step.state = s;

        step.action = action;

        step.reward = reward;

        step.costs = costs;

        step.shock_severity = s.shock_severity;

        step.shock_type = s.shock_type;

        replay.push(step);

        if (done > 0.5) {

            std::cout << "Episode finished at t=" << t << "\n";

            
        }

    }

    replay.save_csv("simulation.csv");

    std::cout << "Saved replay to simulatioin.csv\n";

    std::cout << "Done.\n";

    return 0;

}