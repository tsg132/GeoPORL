#pragma once
#include <vector>
#include <cstddef>

namespace rl {

struct PolicyConfig {

    int state_dim;

    int num_actions;

    float lr;

    float l2;

};

class Policy {

    public:

    explicit Policy(const PolicyConfig& cfg);
    ~Policy();

    // Forward: X[B, D] -> logits[B, A] and probs[B, A]

    // Returns device pointers to probs/lofits for immediate use

    void forward(const float* h_X, int B,
                std::vector<float>& h_logits,
                std::vector<float>& h_probs);

    
    /*
    REINFORCE UPDATE:   

    inputs(host):

    X[B, D], A_idx[B], Adv[B] (advantage or return baseline)

    gradW = X^T * (onehot(A) - probs) * (-Adv) / B 

    W -= lr * (gradW + l2 * W)

    */

    void update(const float* h_X, const int* h_A, const float* h_Adv, int B);

    void act(const float* h_state, bool stochastic=true);

    void get_weights(std::vector<float>& h_W) const;

    void set_weights(const std::vector<float>& h_W);

    int D() const {return cfg_.state_dim;}

    int A() const {return cfg_.num_actions;}

    private:

    PolicyConfig cfg_;

    float* d_W_;

    float* d_X_;

    float* d_logits_;

    float* d_probs_;

    float* d_gradW_;

    float* d_tmp_;

    int cap_B_;

    void ensure_capacity(int B);

    void zero_grad();


};

}