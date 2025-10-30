// #include "geoporl/replay.hpp"

// #include <fstream>

// #include <iomanip>

// void ReplayBuffer::clear() {

//     steps_.clear();

// }

// void ReplayBuffer::push(const ReplayStep& step) {
    
//     steps_.push_back(step);

// }

// const std::vector<ReplayStep>& ReplayBuffer::data() const {
//     return steps_; // Ask this
// }

// void ReplayBuffer::save_csv(const std::string& filename) const {

//     std::ofstream file(filename);

//     file << "episode,t,action,reward,cost0,cost1,cost2,"
//         << "gdp_pc,unemployment,inflation,debt_to_gdp,trade_openness,"
//         << "foreign_direct_investment,economic_complexity,"
//         << "population,education,healthcare,life_expectancy,"
//         << "inequality,urbanization,cohesion,"
//         << "democracy,civil_liberties,corruption,state_capacity,"
//         << "polarization,rule_of_law,"
//         << "military_spend,global_force_projection,nuclear_deterrence,"
//         << "cyber_capability,internal_conflict_risk,external_threat,"
//         << "refugee_pressure,alliance_value,soft_power,diplo_balance,"
//         << "sanction_risk,geo_location_value,autonomy,"
//         << "resource_dependency,rare_earth,water_scarcity,food_security,"
//         << "renewables_share,disaster_vulnerability,"
//         << "rnd_investment,tech_adoption,ai_readiness,brain_drain,patent_output,"
//         << "shock_severity,shock_type\n";

//     for (const auto& s : steps_) {
//         const State& st = s.state;

//         file << s.episode_id << "," << s.t << "," << s.action << ","
//              << s.reward << "," << s.costs[0] << "," << s.costs[1] << "," << s.costs[2] << ","
//              << st.gdp_pc << "," << st.unemployment << "," << st.inflation
//              << "," << st.debt_to_gdp << "," << st.trade_openness
//              << "," << st.foreign_direct_investment << "," << st.economic_complexity
//              << "," << st.population << "," << st.education << "," << st.healthcare
//              << "," << st.life_expectancy << "," << st.inequality
//              << "," << st.urbanization << "," << st.cohesion
//              << "," << st.democracy << "," << st.civil_liberties
//              << "," << st.corruption << "," << st.state_capacity
//              << "," << st.polarization << "," << st.rule_of_law
//              << "," << st.military_spend << "," << st.global_force_projection
//              << "," << st.nuclear_deterrence << "," << st.cyber_capability
//              << "," << st.internal_conflict_risk << "," << st.external_threat
//              << "," << st.refugee_pressure
//              << "," << st.alliance_value << "," << st.soft_power
//              << "," << st.diplo_balance << "," << st.sanction_risk
//              << "," << st.geo_location_value << "," << st.autonomy
//              << "," << st.resource_dependency << "," << st.rare_earth
//              << "," << st.water_scarcity << "," << st.food_security
//              << "," << st.renewables_share << "," << st.disaster_vulnerability
//              << "," << st.rnd_investment << "," << st.tech_adoption
//              << "," << st.ai_readiness << "," << st.brain_drain
//              << "," << st.patent_output
//              << "," << s.shock_severity << "," << s.shock_type
//              << "\n";
//     }

//     file.close();    

// }

#include <vector>
#include <random>
#include <algorithm>
#include "geoporl/state.hpp"

namespace geoporl::rl {

struct Transition {
    State s;
    int   action;
    double reward;
    State s_next;
    bool  done;
};

class ReplayBuffer {
public:
    explicit ReplayBuffer(size_t capacity)
        : cap_(capacity) { buf_.reserve(capacity); }

    void add(const State& s, int action, double reward, const State& s_next, bool done) {
        if (buf_.size() < cap_) {
            buf_.push_back({s, action, reward, s_next, done});
        } else {
            buf_[pos_] = {s, action, reward, s_next, done};
            pos_ = (pos_ + 1) % cap_;
        }
    }

    // Sample indices (caller can fetch items)
    std::vector<size_t> sample_indices(size_t batch, std::mt19937_64& rng) const {
        batch = std::min(batch, buf_.size());
        std::vector<size_t> idx(buf_.size());
        std::iota(idx.begin(), idx.end(), 0);
        std::shuffle(idx.begin(), idx.end(), rng);
        idx.resize(batch);
        return idx;
    }

    const Transition& operator[](size_t i) const { return buf_[i]; }
    size_t size() const { return buf_.size(); }
    bool   full() const { return buf_.size() == cap_; }

private:
    size_t cap_;
    size_t pos_ = 0;
    std::vector<Transition> buf_;
};

} // namespace geoporl::rl