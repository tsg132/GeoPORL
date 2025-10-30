// // #include "geoporl/env.hpp"
// // #include "geoporl/actions.hpp"
// // #include "geoporl/shock.hpp"
// // #include "geoporl/dynamics.hpp"

// // #include <iostream>
// // #include <fstream>
// // #include <sstream>
// // #include <random>
// // #include <iomanip>

// // int main() {

// //     const int NUM_SIMULATIONS = 100;
// //     const int STEPS = 120;
// //     const uint64_t BASE_SEED = 42;

// //     std::cout << "Running " << NUM_SIMULATIONS << " GeoPORL simulations with " << STEPS << " timesteps each...\n";

// //     for (int sim = 0; sim < NUM_SIMULATIONS; sim++) {
        
// //         uint64_t seed = BASE_SEED + sim;
// //         std::mt19937_64 rng(seed);
// //         std::uniform_int_distribution<int> action_dist(1, static_cast<int>(Action::COUNT) - 1);

// //         Env env(seed);
// //         ShockParams shock_params;
// //         geoporl::init_shock_params(shock_params);

// //         env.reset();

// //         // Create CSV filename for this simulation
// //         std::ostringstream filename;
// //         filename << "simulation_" << std::setw(3) << std::setfill('0') << sim << ".csv";
        
// //         std::ofstream csvfile(filename.str());
// //         csvfile << "t,action,reward,cost0,cost1,cost2,"
// //                 << "gdp_pc,cohesion,internal_conflict_risk,shock_severity,shock_type,"
// //                 << "unemployment,inflation,debt_to_gdp,democracy,education,healthcare,"
// //                 << "inequality,polarization,state_capacity,systemic_stress\n";

// //         for (int t = 0; t < STEPS; t++) {
// //             int action = action_dist(rng);

// //             Shock shock = geoporl::sample_shock(env.state(), shock_params, rng);
            
// //             State& s_mutable = const_cast<State&>(env.state());
// //             if (shock.type != ShockType::None) {
// //                 s_mutable.shock_severity = shock.severity;
// //                 s_mutable.shock_type = static_cast<int>(shock.type);
// //                 s_mutable.shock_embed = shock.embed;
// //             }

// //             double reward = 0.0;
// //             std::array<double, 3> costs = {0.0, 0.0, 0.0};
// //             double done = env.step(action, reward, costs);

// //             const State& s = env.state();

// //             csvfile << s.t << "," << action << "," << reward << ","
// //                     << costs[0] << "," << costs[1] << "," << costs[2] << ","
// //                     << s.gdp_pc << "," << s.cohesion << "," << s.internal_conflict_risk << ","
// //                     << s.shock_severity << "," << s.shock_type << ","
// //                     << s.unemployment << "," << s.inflation << "," << s.debt_to_gdp << ","
// //                     << s.democracy << "," << s.education << "," << s.healthcare << ","
// //                     << s.inequality << "," << s.polarization << "," << s.state_capacity << ","
// //                     << s.systemic_stress << "\n";

// //             if (done > 0.5) {
// //                 break;
// //             }
// //         }

// //         csvfile.close();

// //         if ((sim + 1) % 10 == 0) {
// //             std::cout << "Completed " << (sim + 1) << " / " << NUM_SIMULATIONS << " simulations\n";
// //         }
// //     }

// //     std::cout << "\nAll simulations complete. Results saved to simulation_*.csv\n";

// //     return 0;
// // }



// #include "geoporl/env.hpp"
// #include "geoporl/actions.hpp"
// #include "geoporl/shock.hpp"
// #include "geoporl/dynamics.hpp"
// #include "geoporl/replay.hpp"
// #include "geoporl/metrics.hpp"
// #include "rl/policy.hpp"

// #include <iostream>
// #include <fstream>
// #include <random>
// #include <iomanip>
// #include <numeric>
// #include <cmath>

// // === Training hyperparameters ===
// constexpr int EPISODES = 300;
// constexpr int STEPS = 120;          // months (10 years)
// constexpr double GAMMA = 0.99;      // discount
// constexpr size_t REPLAY_CAP = 4096; // large enough for full rollouts
// constexpr int PRINT_INTERVAL = 10;

// int main() {

//     std::cout << "=== GeoPORL Policy Training ===\n";

//     std::mt19937_64 rng(42);

//     Env env(42);

//     ShockParams shock_params;

//     geoporl::init_shock_params(shock_params);

//     rl::PolicyConfig cfg;

//     cfg.state_dim = sizeof(State) / sizeof(double);

//     cfg.num_actions = static_cast<int>(Action::COUNT);

//     cfg.lr = 1e-3f;

//     cfg.l2 = 1e-4f;

//     rl::Policy policy(cfg);

//     geoporl::rl::ReplayBuffer replay(REPLAY_CAP);

//     for (int episode = 0; episode < EPISODES; ++episode) {

//         env.reset();

//         replay = geoporl::rl::ReplayBuffer(REPLAY_CAP);

//         double episode_reward = 0.0;

//         for (int t = 0; t < STEPS; ++t) {
            

//             const State& s = env.state();

//             std::vector<float> s_vec(reinterpret_cast<const float*>(&s), reinterpret_cast<const float*>(&s) + sizeof(State) / sizeof(float));

//             int action = policy.act(s_vec.data(), true);

//             Shock shock = geoporl::sample_shock(s, shock_params, rng);

//             std::array<double, 3> costs = {0.0, 0.0, 0.0};

//             double reward = 0.0;

//             double done = env.step(action, reward, costs);

//             replay.add(s, action, reward, env.state(), done > 0.5);

//             episode_reward += reward;

//             if (done > 0.5) break;
//         }

//         std::vector<float> returns;

//         returns.reserve(replay.size());

//         double G = 0.0;

//         for (int i = replay.size() - 1; i >= 0; --i) {

//             G = replay[i].reward + GAMMA * G * (!replay[i].done);

//             returns.push_back(static_cast<float>(G));
//         }

//         std::reverse(returns.begin(), returns.end());

//         double mean = std::accumulate(returns.begin(), returns.end(), 0.0) / returns.size();

//         double var = 0.0;

//         for (float r: returns) var += (r - mean) * (r - mean);

//         var /= returns.size();

//         double stddev = std::sqrt(var) + 1e-8;

//         for (float& r : returns) r = (r - mean) / stddev;

//         std::vector<float> X;

//         std::vector<int> A;

//         std::vector<float> Adv;

//         X.reserve(replay.size() * cfg.state_dim);

//         A.reserve(replay.size());

//         Adv.reserve(replay.size());

//         for (size_t i = 0; i < replay.size(); ++i) {

//             const State& s = replay[i].s;
            
//             const float* fptr = reinterpret_cast<const float*> (&s);

//             X.insert(X.end(), fptr, fptr + cfg.state_dim);

//             A.push_back(replay[i].action);

//             Adv.push_back(returns[i]);
//         }

//         policy.update(X.data(), A.data(), Adv.data(), replay.size());

//         if ((episode + 1) % PRINT_INTERVAL == 0) {

//             std::cout << "Episode" << std::setw(3) << episode + 1
//             << " | Total reward: " << std::fixed << std::setprecision(3)
//             << episode_reward << std::endl;
//         }




//     }
// }

#include "geoporl/env.hpp"
#include "geoporl/actions.hpp"
#include "geoporl/shock.hpp"
#include "geoporl/dynamics.hpp"
#include "geoporl/replay.hpp"
#include "geoporl/metrics.hpp"
#include "rl/policy.hpp"
#include "geoporl/state_encode.hpp"

#include <iostream>
#include <fstream>
#include <random>
#include <iomanip>
#include <numeric>
#include <cmath>
#include <algorithm>

// === Training hyperparameters ===
constexpr int EPISODES = 300;
constexpr int STEPS = 120;          // months (10 years)
constexpr double GAMMA = 0.99;      // discount
constexpr size_t REPLAY_CAP = 4096; // large enough for full rollouts
constexpr int PRINT_INTERVAL = 10;

int main() {

    std::cout << "=== GeoPORL Policy Training ===\n";

    std::mt19937_64 rng(42);

    Env env(42);

    ShockParams shock_params;

    geoporl::init_shock_params(shock_params);

    rl::PolicyConfig cfg;

    cfg.state_dim = sizeof(State) / sizeof(double);

    cfg.num_actions = static_cast<int>(Action::COUNT);

    cfg.lr = 1e-3f;

    cfg.l2 = 1e-4f;

    rl::Policy policy(cfg);

    geoporl::rl::ReplayBuffer replay(REPLAY_CAP);

    for (int episode = 0; episode < EPISODES; ++episode) {

        env.reset();

        replay = geoporl::rl::ReplayBuffer(REPLAY_CAP);

        double episode_reward = 0.0;

        for (int t = 0; t < STEPS; ++t) {
            

            const State& s = env.state();

            // std::vector<float> s_vec(reinterpret_cast<const float*>(&s), reinterpret_cast<const float*>(&s) + sizeof(State) / sizeof(float));

            auto s_vec = encode_state(s);

            int action = policy.act(s_vec.data(), true);

            Shock shock = geoporl::sample_shock(s, shock_params, rng);

            std::array<double, 3> costs = {0.0, 0.0, 0.0};

            double reward = 0.0;

            double done = env.step(action, reward, costs);

            replay.add(s, action, reward, env.state(), done > 0.5);

            episode_reward += reward;

            if (done > 0.5) break;
        }

        std::vector<float> returns;

        returns.reserve(replay.size());

        double G = 0.0;

        for (int i = replay.size() - 1; i >= 0; --i) {

            G = replay[i].reward + GAMMA * G * (!replay[i].done);

            returns.push_back(static_cast<float>(G));
        }

        std::reverse(returns.begin(), returns.end());

        double mean = std::accumulate(returns.begin(), returns.end(), 0.0) / returns.size();

        double var = 0.0;

        for (float r: returns) var += (r - mean) * (r - mean);

        var /= returns.size();

        double stddev = std::sqrt(var) + 1e-8;

        for (float& r : returns) r = (r - mean) / stddev;

        std::vector<float> X;

        std::vector<int> A;

        std::vector<float> Adv;

        X.reserve(replay.size() * cfg.state_dim);

        A.reserve(replay.size());

        Adv.reserve(replay.size());

        for (size_t i = 0; i < replay.size(); ++i) {

            // const State& s = replay[i].s;
            
            // const float* fptr = reinterpret_cast<const float*> (&s);

            // X.insert(X.end(), fptr, fptr + cfg.state_dim);

            // A.push_back(replay[i].action);

            // Adv.push_back(returns[i]);

              auto s_vec = encode_state(replay[i].s);
              X.insert(X.end(), s_vec.begin(), s_vec.end());
              A.push_back(replay[i].action);
              Adv.push_back(returns[i]);
        }

        if ((episode + 1) % 10 == 0) {
          double w_sum = 0;
          std::vector<float> w;
          policy.get_weights(w);
          for (auto v : w) w_sum += v;
          std::cout << "Episode " << episode+1
              << " | Reward: " << episode_reward
              << " | mean(W): " << (w_sum / w.size()) << "\n";
}

        policy.update(X.data(), A.data(), Adv.data(), replay.size());

        if ((episode + 1) % PRINT_INTERVAL == 0) {

            std::cout << "Episode" << std::setw(3) << episode + 1
            << " | Total reward: " << std::fixed << std::setprecision(3)
            << episode_reward << std::endl;
        }




    }
}