#include "geoporl/replay.hpp"
#include <algorithm>
#include <numeric>   // for std::iota
#include <random>

namespace geoporl::rl {

ReplayBuffer::ReplayBuffer(size_t capacity)
: cap_(capacity) {
    buf_.reserve(capacity);
}

void ReplayBuffer::add(const State& s, int action, double reward,
                       const State& s_next, bool done) {
    if (buf_.size() < cap_) {
        buf_.push_back(Transition{s, action, reward, s_next, done});
    } else {
        buf_[pos_] = Transition{s, action, reward, s_next, done};
        pos_ = (pos_ + 1) % cap_;
    }
}

std::vector<size_t> ReplayBuffer::sample_indices(size_t batch, std::mt19937_64& rng) const {
    const size_t n = buf_.size();
    if (n == 0) return {};
    batch = std::min(batch, n);

    std::vector<size_t> idx(n);
    std::iota(idx.begin(), idx.end(), 0);
    std::shuffle(idx.begin(), idx.end(), rng);
    idx.resize(batch);
    return idx;
}

const Transition& ReplayBuffer::operator[](size_t i) const {
    return buf_[i];
}

size_t ReplayBuffer::size() const {
    return buf_.size();
}

bool ReplayBuffer::full() const {
    return buf_.size() == cap_;
}

} // namespace geoporl::rl