#include <cuda_runtime.h>
#include <curand_kernel.h>
#include <cublas_v2.h>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include "rl/policy.hpp"

// ======== Error Handling Helpers ======== //
#define CUDA_OK(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, const char* file, int line) {
    if (code != cudaSuccess)
        throw std::runtime_error(std::string("CUDA error: ") +
            cudaGetErrorString(code) + " at " + file + ":" + std::to_string(line));
}

#define CUBLAS_OK(ans) { cublasAssert((ans), __FILE__, __LINE__); }
inline void cublasAssert(cublasStatus_t code, const char* file, int line) {
    if (code != CUBLAS_STATUS_SUCCESS)
        throw std::runtime_error(std::string("cuBLAS error at ") +
            file + ":" + std::to_string(line));
}


/*

we'll implement a lienar softmax policy: pi(a|s) = softmax(W^{T}s) with W in [D, A]

Forward pass:

1) logits = XW (batched states X in [B, D], cuBLAS computes column major)

2) probs = softmax(logits )

*/


namespace rl {

static cublasHandle_t g_blasl

__global__ void softmax_rowwise(float* logits, float* probs, int B, int A) {

    int b = blockIdx.x * blockDim.x + threadIdx.x;

    if (b >= B) return;

    float* L = logits + b * A;

    float* P = probs + b * A;

    float m = L[0];

    for (int i = 1; i < A; ++i) m = fmaxf(m, L[i]);

    float s = 0.f;

    for (int i = 0; i < A; ++i) {

        float e = expf(L[i] - m);

        P[i] = e;

        s += e;
    }

    float inv = 1.f / s;

    for (int i = 0; i < A; ++i) P[i] *= inv;

}

__global__ void probs_minus_onehot_scaled(float* probs, const int* A_idx, const float* Adv, int B, int A) {

    int idx = blockIdx.x * blockDim.x + threadIdx.x;

    if (idx >= B * A) return;

    int b = idx / A;

    int a = idx % A;

    float p = probs[idx];

    int a_taken = A_idx[b];

    float g = p - (a == a_taken ? 1.f : 0.f);

    float scale = -Adv[b];

    probs[idx] = g * scale

}

__global__ void l2_add(float* gradW, const float* W, float l2, int N) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < N)
        gradW[i] += l2 * W[i];
}

__global__ void sgd(float* W, const float* gradW, float lr, int N) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < N)
        W[i] -= lr * gradW[i];
}

Policy::Policy(const PolicyConfig& cfg)
    : cfg_(cfg),
      d_W_(nullptr), d_X_(nullptr), d_logits_(nullptr), d_probs_(nullptr),
      d_gradW_(nullptr), d_tmp_(nullptr), cap_B_(0)
    {
    static bool blas_inited = false;
    if (!blas_inited) {
        CUBLAS_OK(cublasCreate(&g_blas));
        blas_inited = true;
    }

    // Xavier initialization
    std::vector<float> h_W(cfg_.state_dim * cfg_.num_actions);
    float scale = std::sqrt(2.0f / (cfg_.state_dim + cfg_.num_actions));
    for (auto& w : h_W)
        w = scale * (float(rand()) / RAND_MAX - 0.5f);

    CUDA_OK(cudaMalloc(&d_W_, sizeof(float) * h_W.size()));
    CUDA_OK(cudaMemcpy(d_W_, h_W.data(), sizeof(float) * h_W.size(), cudaMemcpyHostToDevice));
}

Policy::~Policy() {
    cudaFree(d_W_);
    cudaFree(d_X_);
    cudaFree(d_logits_);
    cudaFree(d_probs_);
    cudaFree(d_gradW_);
    cudaFree(d_tmp_);
}

// Ensure sufficient device memory for batch size B
void Policy::ensure_capacity(int B) {
    if (B <= cap_B_) return;
    cap_B_ = B;

    cudaFree(d_X_);
    cudaFree(d_logits_);
    cudaFree(d_probs_);
    cudaFree(d_gradW_);
    cudaFree(d_tmp_);

    CUDA_OK(cudaMalloc(&d_X_,      sizeof(float) * B * cfg_.state_dim));
    CUDA_OK(cudaMalloc(&d_logits_, sizeof(float) * B * cfg_.num_actions));
    CUDA_OK(cudaMalloc(&d_probs_,  sizeof(float) * B * cfg_.num_actions));
    CUDA_OK(cudaMalloc(&d_gradW_,  sizeof(float) * cfg_.state_dim * cfg_.num_actions));
    CUDA_OK(cudaMalloc(&d_tmp_,    sizeof(float) * B)); // spare
}

void Policy::zero_grad() {

    size_t N = size_t(cfg_.state_dim) * cfg_.num_actions;

    CUDA_OK(cudaMemset(d_gradW_, 0, sizeof(float) * N));

}

void Policy::forward(const float* h_X, int B,
                    std::vector<float>& h_logits,
                    std::vector<float>& h_probs) 
                    
{

    ensure_capacity(B);

    CUDA_OK(cudaMemcpy(d_X_, h_X, sizeof(float) * B * cfg_.state_dim, cudaMemcpyHostToDevice));

    const float alpha = 1.0f, beta = 0.0f;

    CUBLAS_OK(cublasSgemm(g_blas,
        CUBLAS_OP_N, CUBLAS_OP_N,
        cfg_.num_actions, B, cfg_.state_dim,
        &alpha,
        d_W_, cfg_.num_actions,
        d_X_, cfg_.state_dim,
        &beta,
        d_logits_, cfg_.num_actions));

    int threads = 256;

    int blocks = (B + threads - 1) / threads;

    softmax_rowwise<<<blocks, threads>>>(d_logits_, d_probs_, B, cfg_num_actions);

    CUDA_OK(cudaPeekAtLastError());
    
}  

}