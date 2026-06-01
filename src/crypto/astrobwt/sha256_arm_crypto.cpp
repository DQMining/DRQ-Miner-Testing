/* ARMv8 SHA256 using crypto extensions (requires -march=armv8-a+crypto / XMRIG_ARM_CRYPTO). */
#ifdef XMRIG_ARM_CRYPTO

#include "crypto/astrobwt/sha256_utils.h"

#include <arm_neon.h>
#include <cstring>

namespace xmrig {
namespace astrobwt {

static const uint32_t K256[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
};

static inline uint32x4_t sha256_load_be32(const uint32_t *src)
{
    return vreinterpretq_u32_u8(vrev32q_u8(vld1q_u8(reinterpret_cast<const uint8_t *>(src))));
}

static void sha256_arm_transform(uint32_t state[8], const uint8_t block[64])
{
    uint32x4_t abcd = vld1q_u32(&state[0]);
    uint32x4_t efgh = vld1q_u32(&state[4]);

    uint32x4_t abcd_orig = abcd;
    uint32x4_t efgh_orig = efgh;

    uint32x4_t w0 = sha256_load_be32(reinterpret_cast<const uint32_t *>(block + 0));
    uint32x4_t w1 = sha256_load_be32(reinterpret_cast<const uint32_t *>(block + 16));
    uint32x4_t w2 = sha256_load_be32(reinterpret_cast<const uint32_t *>(block + 32));
    uint32x4_t w3 = sha256_load_be32(reinterpret_cast<const uint32_t *>(block + 48));

    uint32x4_t k0 = vld1q_u32(&K256[0]);
    uint32x4_t k1 = vld1q_u32(&K256[4]);
    uint32x4_t k2 = vld1q_u32(&K256[8]);
    uint32x4_t k3 = vld1q_u32(&K256[12]);

    abcd = vsha256hq_u32(abcd, efgh, vaddq_u32(w0, k0));
    efgh = vsha256h2q_u32(efgh, abcd, vaddq_u32(w0, k0));
    w0 = vsha256su1q_u32(vsha256su0q_u32(w0, w1), w2, w3);

    abcd = vsha256hq_u32(abcd, efgh, vaddq_u32(w1, k1));
    efgh = vsha256h2q_u32(efgh, abcd, vaddq_u32(w1, k1));
    w1 = vsha256su1q_u32(vsha256su0q_u32(w1, w2), w3, w0);

    abcd = vsha256hq_u32(abcd, efgh, vaddq_u32(w2, k2));
    efgh = vsha256h2q_u32(efgh, abcd, vaddq_u32(w2, k2));
    w2 = vsha256su1q_u32(vsha256su0q_u32(w2, w3), w0, w1);

    abcd = vsha256hq_u32(abcd, efgh, vaddq_u32(w3, k3));
    efgh = vsha256h2q_u32(efgh, abcd, vaddq_u32(w3, k3));
    w3 = vsha256su1q_u32(vsha256su0q_u32(w3, w0), w1, w2);

    for (int t = 16; t < 64; t += 16) {
        k0 = vld1q_u32(&K256[t]);
        k1 = vld1q_u32(&K256[t + 4]);
        k2 = vld1q_u32(&K256[t + 8]);
        k3 = vld1q_u32(&K256[t + 12]);

        abcd = vsha256hq_u32(abcd, efgh, vaddq_u32(w0, k0));
        efgh = vsha256h2q_u32(efgh, abcd, vaddq_u32(w0, k0));
        w0 = vsha256su1q_u32(vsha256su0q_u32(w0, w1), w2, w3);

        abcd = vsha256hq_u32(abcd, efgh, vaddq_u32(w1, k1));
        efgh = vsha256h2q_u32(efgh, abcd, vaddq_u32(w1, k1));
        w1 = vsha256su1q_u32(vsha256su0q_u32(w1, w2), w3, w0);

        abcd = vsha256hq_u32(abcd, efgh, vaddq_u32(w2, k2));
        efgh = vsha256h2q_u32(efgh, abcd, vaddq_u32(w2, k2));
        w2 = vsha256su1q_u32(vsha256su0q_u32(w2, w3), w0, w1);

        abcd = vsha256hq_u32(abcd, efgh, vaddq_u32(w3, k3));
        efgh = vsha256h2q_u32(efgh, abcd, vaddq_u32(w3, k3));
        w3 = vsha256su1q_u32(vsha256su0q_u32(w3, w0), w1, w2);
    }

    abcd = vaddq_u32(abcd, abcd_orig);
    efgh = vaddq_u32(efgh, efgh_orig);

    vst1q_u32(&state[0], abcd);
    vst1q_u32(&state[4], efgh);
}

void sha256_arm_ni(const uint8_t *data, size_t len, uint8_t hash[32])
{
    uint32_t state[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };

    const size_t full_blocks = len / 64;
    for (size_t i = 0; i < full_blocks; ++i) {
        sha256_arm_transform(state, data + i * 64);
    }

    uint8_t pad[128];
    memset(pad, 0, sizeof(pad));
    const size_t rem = len - full_blocks * 64;
    memcpy(pad, data + full_blocks * 64, rem);
    pad[rem] = 0x80;

    const size_t pad_blocks = (rem < 56) ? 1U : 2U;
    const uint64_t bitlen = len * 8U;
    const size_t end = pad_blocks * 64U - 8U;
    for (int i = 0; i < 8; ++i) {
        pad[end + static_cast<size_t>(i)] = static_cast<uint8_t>(bitlen >> (56 - i * 8));
    }

    for (size_t i = 0; i < pad_blocks; ++i) {
        sha256_arm_transform(state, pad + i * 64);
    }

    for (int i = 0; i < 8; ++i) {
        hash[i * 4]     = static_cast<uint8_t>(state[i] >> 24);
        hash[i * 4 + 1] = static_cast<uint8_t>(state[i] >> 16);
        hash[i * 4 + 2] = static_cast<uint8_t>(state[i] >> 8);
        hash[i * 4 + 3] = static_cast<uint8_t>(state[i]);
    }
}

} // namespace astrobwt
} // namespace xmrig

#endif
