/* VAES-256 scratch fill — isolated TU for MSVC (clang-cl -mvaes -mavx2). */
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC optimize("no-tree-vectorize")
#endif
#if defined(__GNUC__) || defined(__clang__)
#   define NM_TARGET(x) __attribute__((target(x)))
#else
#   define NM_TARGET(x)
#endif

#include "nm_fast_fill.h"
#include "nm_params.h"
#include "nm_sha256.h"
#include "nm_aes.h"

#include <string.h>

#if defined(_MSC_VER)
#   include <immintrin.h>
#else
#   include <x86intrin.h>
#endif

NM_TARGET("avx2,vaes")
void nm_fast_fill_vaes256(const nm_epoch *e, nm_lane *l, const uint8_t seed[32])
{
    (void) e;
    uint8_t key[32], t[33];
    memcpy(t, seed, 32);
    t[32] = 0x53;
    nm_sha256(t, 33, key);
    nm_aes128 a;
    nm_aes128_expand(&a, key);
    __m256i rk[11];
    for (int r = 0; r < 11; r++) {
        rk[r] = _mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i *) (a.rk + 16 * r)));
    }
    uint64_t khi;
    memcpy(&khi, key + 24, 8);
    long long kh = (long long) khi;
    uint64_t *sc = l->scratch;
    for (uint32_t i = 0; i < NM_SCRATCH_WORDS; i += 16) {
        __m256i y0 = _mm256_xor_si256(_mm256_set_epi64x(kh, (long long) (uint64_t) (i + 2), kh, (long long) (uint64_t) (i + 0)), rk[0]);
        __m256i y1 = _mm256_xor_si256(_mm256_set_epi64x(kh, (long long) (uint64_t) (i + 6), kh, (long long) (uint64_t) (i + 4)), rk[0]);
        __m256i y2 = _mm256_xor_si256(_mm256_set_epi64x(kh, (long long) (uint64_t) (i + 10), kh, (long long) (uint64_t) (i + 8)), rk[0]);
        __m256i y3 = _mm256_xor_si256(_mm256_set_epi64x(kh, (long long) (uint64_t) (i + 14), kh, (long long) (uint64_t) (i + 12)), rk[0]);
        for (int r = 1; r < 10; r++) {
            __m256i k = rk[r];
            y0 = _mm256_aesenc_epi128(y0, k);
            y1 = _mm256_aesenc_epi128(y1, k);
            y2 = _mm256_aesenc_epi128(y2, k);
            y3 = _mm256_aesenc_epi128(y3, k);
        }
        __m256i kl = rk[10];
        _mm256_storeu_si256((__m256i *) &sc[i + 0], _mm256_aesenclast_epi128(y0, kl));
        _mm256_storeu_si256((__m256i *) &sc[i + 4], _mm256_aesenclast_epi128(y1, kl));
        _mm256_storeu_si256((__m256i *) &sc[i + 8], _mm256_aesenclast_epi128(y2, kl));
        _mm256_storeu_si256((__m256i *) &sc[i + 12], _mm256_aesenclast_epi128(y3, kl));
    }
}
