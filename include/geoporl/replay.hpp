#pragma once

#include "state.hpp"

#include <vector>

#include <array>

#include <string>

#include <random>


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

};

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
    explicit ReplayBuffer(size_t capacity);

    void add(const State& s, int action, double reward, const State& s_next, bool done);

    std::vector<size_t> sample_indices(size_t batch, std::mt19937_64& rng) const;

    const Transition& operator[](size_t i) const;
    size_t size() const;
    bool   full() const;

private:
    size_t cap_;
    size_t pos_ = 0;
    std::vector<Transition> buf_;
};

} // namespace geoporl::rl