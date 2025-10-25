#pragma once

#include <random>

class RNG {

    public:

        RNG(uint64_t seed = 0) : eng(seed) {}

        double uniform(double a = 0.0, double b = 1.0) {

            std::uniform_real_distribution<double> d(a, b);
            return d(eng);

        }
    
    private:

        std::mt19937_64 eng;

};