#pragma once

#include "state.hpp"

#include <vector>

#include <array>

#include <string>


struct ReplayStep {
    int episode_id;
    int t;
    State state;
    int action;
    double reward;
    std::array<double,3> costs;
    float shock_severity;
    int shock_type;
};

class ReplayBuffer {

    public:

        ReplayBuffer() = default;

        void clear();

        void push(const ReplayStep& step);

        const std::vector<ReplayStep>& data() const;

        void save_csv(const std::string& filename) const;

    private:

        std::vector<ReplayStep> steps_;

};  // Ask about the construction of ReplayBuffer