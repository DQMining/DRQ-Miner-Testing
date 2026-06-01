#pragma once
/* Minimal replacements for ccminer cuda_helper.h / miner.h when compiling verus GPU TU. */
#include <cstdio>
#include <cuda_runtime.h>

#ifndef CUDA_SAFE_CALL
#   define CUDA_SAFE_CALL(call)                                                                 \
        do {                                                                                    \
            cudaError_t _err = (call);                                                          \
            if (_err != cudaSuccess) {                                                          \
                fprintf(stderr, "CUDA error %s:%d: %s\n", __FILE__, __LINE__,                   \
                        cudaGetErrorString(_err));                                              \
            }                                                                                   \
        } while (0)
#endif

/* Recent nvcc (e.g. CUDA 13): uint2/uint4 are plain structs — no built-in ^, &, |. */
#if defined(__CUDACC__)
#   define AS_UINT4(r_)                                                                                 \
        make_uint4(                                                                                     \
            (uint32_t)((const uint64_t *)(r_))[0],                                                      \
            (uint32_t)(((const uint64_t *)(r_))[0] >> 32U),                                             \
            (uint32_t)((const uint64_t *)(r_))[1],                                                      \
            (uint32_t)(((const uint64_t *)(r_))[1] >> 32U))

#   define _mm_xor_si128_emu(a_, b_)                                                                   \
        make_uint4((a_).x ^ (b_).x, (a_).y ^ (b_).y, (a_).z ^ (b_).z, (a_).w ^ (b_).w)

#   define xor3(a_, b_, c_) (((a_) ^ (b_)) ^ (c_))

/** 64-bit rotate-right of (x=low, y=high) — ccminer cuda_helper ROR2. */
__device__ __forceinline__ uint2 ROR2(uint2 v, unsigned int n)
{
    n &= 63U;
    uint64_t w = static_cast<uint64_t>(v.x) | (static_cast<uint64_t>(v.y) << 32);
    w          = (w >> n) | (w << ((64U - n) & 63U));
    return make_uint2(static_cast<uint32_t>(w), static_cast<uint32_t>(w >> 32));
}
#endif
