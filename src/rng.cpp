// #include "geoporl/rng.hpp"
// #include <algorithm>
// #include <cassert>

// namespace geoporl {

// RNG::RNG(uint64_t seed) : eng_(seed) {}

// void RNG::reseed(uint64_t seed) {
//     eng_.seed(seed);
// }

// double RNG::uniform(double a, double b) {
//     std::uniform_real_distribution<double> dist(a, b);
//     return dist(eng_);
// }

// int RNG::uniform_int(int lo, int hi) {
//     std::uniform_int_distribution<int> dist(lo, hi);
//     return dist(eng_);
// }

// double RNG::normal(double mean, double stddev) {
//     std::normal_distribution<double> dist(mean, stddev);
//     return dist(eng_);
// }

// size_t RNG::random_index(size_t n) {
//     assert(n > 0);
//     std::uniform_int_distribution<size_t> dist(0, n - 1);
//     return dist(eng_);
// }

// } // namespace geoporl