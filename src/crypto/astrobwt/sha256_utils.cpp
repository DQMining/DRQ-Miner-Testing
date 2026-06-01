#include "crypto/astrobwt/sha256_utils.h"

#ifdef _MSC_VER
#include <intrin.h>
#elif !defined(XMRIG_ARM)
#include <x86intrin.h>
#include <cpuid.h>
#endif

namespace xmrig {
namespace astrobwt {

static bool s_sha_ni = false;

static bool detect_sha_ni()
{
#ifdef _MSC_VER
    int info[4];
    __cpuidex(info, 7, 0);
    return (info[1] >> 29) & 1;
#elif !defined(XMRIG_ARM)
    unsigned int eax, ebx, ecx, edx;
    if (__get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx)) {
        return (ebx >> 29) & 1;
    }
    return false;
#else
    return false;
#endif
}

void sha256_init_hw()
{
#   if defined(XMRIG_ARM_CRYPTO)
    s_sha_ni = true;
#   elif defined(__aarch64__)
    s_sha_ni = false;
#   else
    s_sha_ni = detect_sha_ni();
#   endif
}

bool has_sha_ni()
{
    return s_sha_ni;
}

#if defined(XMRIG_ARM_CRYPTO)

void sha256_ni(const uint8_t *data, size_t len, uint8_t hash[32])
{
    sha256_arm_ni(data, len, hash);
}

#elif defined(__SHA__) || defined(_MSC_VER)

#if defined(_MSC_VER)
#   define XMRIG_ALIGNED16 alignas(16)
#else
#   define XMRIG_ALIGNED16 __attribute__((aligned(16)))
#endif

XMRIG_ALIGNED16 static const uint32_t K256[64] = {
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

static inline void sha256_ni_transform(uint32_t state[8], const uint8_t data[], size_t num_blocks)
{
    const __m128i MASK = _mm_set_epi64x(0x0c0d0e0f08090a0bULL, 0x0405060700010203ULL);

    __m128i STATE0 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&state[0]));
    __m128i STATE1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&state[4]));

    __m128i TMP = _mm_shuffle_epi32(STATE0, 0xB1);
    STATE1 = _mm_shuffle_epi32(STATE1, 0x1B);
    STATE0 = _mm_alignr_epi8(TMP, STATE1, 8);
    STATE1 = _mm_blend_epi16(STATE1, TMP, 0xF0);

    while (num_blocks > 0) {
        __m128i ABEF_SAVE = STATE0;
        __m128i CDGH_SAVE = STATE1;

        __m128i MSG0 = _mm_shuffle_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(data + 0)), MASK);
        __m128i MSG1 = _mm_shuffle_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(data + 16)), MASK);
        __m128i MSG2 = _mm_shuffle_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(data + 32)), MASK);
        __m128i MSG3 = _mm_shuffle_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(data + 48)), MASK);
        __m128i MSG;

        // Rounds 0-3
        MSG = _mm_add_epi32(MSG0, _mm_load_si128(reinterpret_cast<const __m128i*>(&K256[0])));
        STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
        MSG = _mm_shuffle_epi32(MSG, 0x0E);
        STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);

        // Rounds 4-7
        MSG = _mm_add_epi32(MSG1, _mm_load_si128(reinterpret_cast<const __m128i*>(&K256[4])));
        STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
        MSG = _mm_shuffle_epi32(MSG, 0x0E);
        STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);
        MSG0 = _mm_sha256msg1_epu32(MSG0, MSG1);

        // Rounds 8-11
        MSG = _mm_add_epi32(MSG2, _mm_load_si128(reinterpret_cast<const __m128i*>(&K256[8])));
        STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
        MSG = _mm_shuffle_epi32(MSG, 0x0E);
        STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);
        MSG1 = _mm_sha256msg1_epu32(MSG1, MSG2);

        // Rounds 12-15
        MSG = _mm_add_epi32(MSG3, _mm_load_si128(reinterpret_cast<const __m128i*>(&K256[12])));
        STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
        MSG = _mm_shuffle_epi32(MSG, 0x0E);
        STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);
        MSG0 = _mm_add_epi32(MSG0, _mm_alignr_epi8(MSG3, MSG2, 4));
        MSG0 = _mm_sha256msg2_epu32(MSG0, MSG3);
        MSG2 = _mm_sha256msg1_epu32(MSG2, MSG3);

        // Rounds 16-19
        MSG = _mm_add_epi32(MSG0, _mm_load_si128(reinterpret_cast<const __m128i*>(&K256[16])));
        STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
        MSG = _mm_shuffle_epi32(MSG, 0x0E);
        STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);
        MSG1 = _mm_add_epi32(MSG1, _mm_alignr_epi8(MSG0, MSG3, 4));
        MSG1 = _mm_sha256msg2_epu32(MSG1, MSG0);
        MSG3 = _mm_sha256msg1_epu32(MSG3, MSG0);

        // Rounds 20-23
        MSG = _mm_add_epi32(MSG1, _mm_load_si128(reinterpret_cast<const __m128i*>(&K256[20])));
        STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
        MSG = _mm_shuffle_epi32(MSG, 0x0E);
        STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);
        MSG2 = _mm_add_epi32(MSG2, _mm_alignr_epi8(MSG1, MSG0, 4));
        MSG2 = _mm_sha256msg2_epu32(MSG2, MSG1);
        MSG0 = _mm_sha256msg1_epu32(MSG0, MSG1);

        // Rounds 24-27
        MSG = _mm_add_epi32(MSG2, _mm_load_si128(reinterpret_cast<const __m128i*>(&K256[24])));
        STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
        MSG = _mm_shuffle_epi32(MSG, 0x0E);
        STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);
        MSG3 = _mm_add_epi32(MSG3, _mm_alignr_epi8(MSG2, MSG1, 4));
        MSG3 = _mm_sha256msg2_epu32(MSG3, MSG2);
        MSG1 = _mm_sha256msg1_epu32(MSG1, MSG2);

        // Rounds 28-31
        MSG = _mm_add_epi32(MSG3, _mm_load_si128(reinterpret_cast<const __m128i*>(&K256[28])));
        STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
        MSG = _mm_shuffle_epi32(MSG, 0x0E);
        STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);
        MSG0 = _mm_add_epi32(MSG0, _mm_alignr_epi8(MSG3, MSG2, 4));
        MSG0 = _mm_sha256msg2_epu32(MSG0, MSG3);
        MSG2 = _mm_sha256msg1_epu32(MSG2, MSG3);

        // Rounds 32-35
        MSG = _mm_add_epi32(MSG0, _mm_load_si128(reinterpret_cast<const __m128i*>(&K256[32])));
        STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
        MSG = _mm_shuffle_epi32(MSG, 0x0E);
        STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);
        MSG1 = _mm_add_epi32(MSG1, _mm_alignr_epi8(MSG0, MSG3, 4));
        MSG1 = _mm_sha256msg2_epu32(MSG1, MSG0);
        MSG3 = _mm_sha256msg1_epu32(MSG3, MSG0);

        // Rounds 36-39
        MSG = _mm_add_epi32(MSG1, _mm_load_si128(reinterpret_cast<const __m128i*>(&K256[36])));
        STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
        MSG = _mm_shuffle_epi32(MSG, 0x0E);
        STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);
        MSG2 = _mm_add_epi32(MSG2, _mm_alignr_epi8(MSG1, MSG0, 4));
        MSG2 = _mm_sha256msg2_epu32(MSG2, MSG1);
        MSG0 = _mm_sha256msg1_epu32(MSG0, MSG1);

        // Rounds 40-43
        MSG = _mm_add_epi32(MSG2, _mm_load_si128(reinterpret_cast<const __m128i*>(&K256[40])));
        STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
        MSG = _mm_shuffle_epi32(MSG, 0x0E);
        STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);
        MSG3 = _mm_add_epi32(MSG3, _mm_alignr_epi8(MSG2, MSG1, 4));
        MSG3 = _mm_sha256msg2_epu32(MSG3, MSG2);
        MSG1 = _mm_sha256msg1_epu32(MSG1, MSG2);

        // Rounds 44-47
        MSG = _mm_add_epi32(MSG3, _mm_load_si128(reinterpret_cast<const __m128i*>(&K256[44])));
        STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
        MSG = _mm_shuffle_epi32(MSG, 0x0E);
        STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);
        MSG0 = _mm_add_epi32(MSG0, _mm_alignr_epi8(MSG3, MSG2, 4));
        MSG0 = _mm_sha256msg2_epu32(MSG0, MSG3);
        MSG2 = _mm_sha256msg1_epu32(MSG2, MSG3);

        // Rounds 48-51
        MSG = _mm_add_epi32(MSG0, _mm_load_si128(reinterpret_cast<const __m128i*>(&K256[48])));
        STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
        MSG = _mm_shuffle_epi32(MSG, 0x0E);
        STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);
        MSG1 = _mm_add_epi32(MSG1, _mm_alignr_epi8(MSG0, MSG3, 4));
        MSG1 = _mm_sha256msg2_epu32(MSG1, MSG0);
        MSG3 = _mm_sha256msg1_epu32(MSG3, MSG0);

        // Rounds 52-55
        MSG = _mm_add_epi32(MSG1, _mm_load_si128(reinterpret_cast<const __m128i*>(&K256[52])));
        STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
        MSG = _mm_shuffle_epi32(MSG, 0x0E);
        STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);
        MSG2 = _mm_add_epi32(MSG2, _mm_alignr_epi8(MSG1, MSG0, 4));
        MSG2 = _mm_sha256msg2_epu32(MSG2, MSG1);

        // Rounds 56-59
        MSG = _mm_add_epi32(MSG2, _mm_load_si128(reinterpret_cast<const __m128i*>(&K256[56])));
        STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
        MSG = _mm_shuffle_epi32(MSG, 0x0E);
        STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);
        MSG3 = _mm_add_epi32(MSG3, _mm_alignr_epi8(MSG2, MSG1, 4));
        MSG3 = _mm_sha256msg2_epu32(MSG3, MSG2);

        // Rounds 60-63
        MSG = _mm_add_epi32(MSG3, _mm_load_si128(reinterpret_cast<const __m128i*>(&K256[60])));
        STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
        MSG = _mm_shuffle_epi32(MSG, 0x0E);
        STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);

        STATE0 = _mm_add_epi32(STATE0, ABEF_SAVE);
        STATE1 = _mm_add_epi32(STATE1, CDGH_SAVE);

        data += 64;
        num_blocks--;
    }

    TMP = _mm_shuffle_epi32(STATE0, 0x1B);
    STATE1 = _mm_shuffle_epi32(STATE1, 0xB1);
    STATE0 = _mm_blend_epi16(TMP, STATE1, 0xF0);
    STATE1 = _mm_alignr_epi8(STATE1, TMP, 8);

    _mm_storeu_si128(reinterpret_cast<__m128i*>(&state[0]), STATE0);
    _mm_storeu_si128(reinterpret_cast<__m128i*>(&state[4]), STATE1);
}

void sha256_ni(const uint8_t *data, size_t len, uint8_t hash[32])
{
    uint32_t state[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };

    size_t full_blocks = len / 64;

    if (full_blocks > 0) {
        sha256_ni_transform(state, data, full_blocks);
        data += full_blocks * 64;
        len -= full_blocks * 64;
    }

    uint8_t pad[128];
    memset(pad, 0, sizeof(pad));
    memcpy(pad, data, len);
    pad[len] = 0x80;

    size_t pad_blocks;
    if (len < 56) {
        pad_blocks = 1;
    } else {
        pad_blocks = 2;
    }

    uint64_t bitlen = (full_blocks * 64 + len) * 8;
    uint8_t *end = pad + pad_blocks * 64 - 8;
    end[0] = (uint8_t)(bitlen >> 56);
    end[1] = (uint8_t)(bitlen >> 48);
    end[2] = (uint8_t)(bitlen >> 40);
    end[3] = (uint8_t)(bitlen >> 32);
    end[4] = (uint8_t)(bitlen >> 24);
    end[5] = (uint8_t)(bitlen >> 16);
    end[6] = (uint8_t)(bitlen >> 8);
    end[7] = (uint8_t)(bitlen);

    sha256_ni_transform(state, pad, pad_blocks);

    const __m128i MASK = _mm_set_epi64x(0x0c0d0e0f08090a0bULL, 0x0405060700010203ULL);
    __m128i s0 = _mm_shuffle_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&state[0])), MASK);
    __m128i s1 = _mm_shuffle_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&state[4])), MASK);
    _mm_storeu_si128(reinterpret_cast<__m128i*>(hash), s0);
    _mm_storeu_si128(reinterpret_cast<__m128i*>(hash + 16), s1);
}

#else

void sha256_ni(const uint8_t *data, size_t len, uint8_t hash[32])
{
    sha256(data, len, hash);
}

#endif

} // namespace astrobwt
} // namespace xmrig
