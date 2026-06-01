#pragma once

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

/* CPU-side preimage steps matching monkins1010/ccminer verusscan.cpp (portable Haraka). */
void verus_cuda_VerusHashHalf(void *result64, const unsigned char *data, int len);
void verus_cuda_GenNewCLKey(const unsigned char *seed32, void *key8832);

#ifdef __cplusplus
}
#endif

#if defined(__cplusplus) && defined(XMRIG_FEATURE_CUDA_VERUS)
extern "C" {
/** 0 on success, -1 on failure (e.g. cudaMalloc). */
int xmrig_verus_cuda_init(int thr_id, uint32_t throughput);
void xmrig_verus_cuda_set_block(uint8_t *blockf, uint32_t *pTargetIn, uint8_t *lkey, int thr_id, uint32_t throughput);
void xmrig_verus_cuda_hash(int thr_id, uint32_t threads, uint32_t startNonce, uint32_t *resNonces, uint8_t version);
void xmrig_verus_cuda_cleanup(int thr_id);
}
#endif
