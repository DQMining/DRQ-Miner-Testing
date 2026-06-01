/* Fast suffix-array build (derohe astrobwtv3/sa_fast.go sort_indices).
 * Miner-only path; output must match text_32_0alloc / divsufsort for valid shares.
 */

#ifndef XMRIG_ASTROBWT_SA_FAST_H
#define XMRIG_ASTROBWT_SA_FAST_H

#include "crypto/astrobwt/AstroBWT.h"

#include <cstdint>
#include <cstring>

namespace xmrig {
namespace astrobwt {

static inline uint32_t read_be_u32(const uint8_t* p)
{
    return (static_cast<uint32_t>(p[0]) << 24) |
           (static_cast<uint32_t>(p[1]) << 16) |
           (static_cast<uint32_t>(p[2]) << 8) |
           static_cast<uint32_t>(p[3]);
}

static inline void sa_fast_fix(const uint8_t* v, uint32_t* indices, int i)
{
    uint32_t prev_t = indices[i];
    uint32_t t = indices[i + 1];

    const uint32_t data_a = read_be_u32(v + ((t) & 0xffff) + 2);
    if (data_a <= read_be_u32(v + ((prev_t) & 0xffff) + 2)) {
        const uint32_t t2 = t;
        int j = i;
        for (;;) {
            indices[j + 1] = prev_t;
            j--;
            if (j < 0) {
                break;
            }
            prev_t = indices[j];
            if (((t ^ prev_t) <= 0xffff) && data_a <= read_be_u32(v + ((prev_t) & 0xffff) + 2)) {
                continue;
            }
            break;
        }
        indices[j + 1] = t;
        (void)t2;
    }
}

static inline void build_suffix_array_fast(uint8_t* v, int32_t* sa, uint32_t n, uint32_t* indices, uint32_t* tmp_indices)
{
    if (n == 0) {
        return;
    }
    if (n == 1) {
        sa[0] = 0;
        return;
    }

    uint16_t byte_counters[2][256];
    uint16_t counters[2][256];

    v[n]     = 0;
    v[n + 1] = 0;

    std::memset(byte_counters, 0, sizeof(byte_counters));

    for (uint32_t i = 0; i < n; i++) {
        byte_counters[1][v[i]]++;
    }

    byte_counters[0][0] = byte_counters[1][0];
    byte_counters[0][v[0]]--;

    counters[0][0] = byte_counters[0][0];
    counters[1][0] = static_cast<uint16_t>(byte_counters[1][0] - 1);

    uint16_t c0 = counters[0][0];
    uint16_t c1 = counters[1][0];

    for (int i = 1; i < 256; i++) {
        c0 = static_cast<uint16_t>(c0 + byte_counters[0][i]);
        c1 = static_cast<uint16_t>(c1 + byte_counters[1][i]);
        counters[0][i] = c0;
        counters[1][i] = c1;
    }

    uint16_t* counters0 = counters[0];
    {
        const uint32_t byte0 = v[n - 1];
        tmp_indices[counters0[0]] = (byte0 << 24) | (n - 1);
        counters0[0]--;
    }

    for (int i = static_cast<int>(n) - 1; i >= 1; i--) {
        const uint32_t byte0 = v[i - 1];
        const uint32_t byte1 = v[i];
        tmp_indices[counters0[v[i]]] = (byte0 << 24) | (byte1 << 16) | static_cast<uint32_t>(i - 1);
        counters0[v[i]]--;
    }

    uint16_t* counters1 = counters[1];
    for (int i = static_cast<int>(n) - 1; i >= 0; i--) {
        const uint32_t data = tmp_indices[i];
        const uint16_t tmp = counters1[data >> 24];
        counters1[data >> 24]--;
        indices[tmp] = data;
    }

    for (uint32_t i = 1; i < n; i++) {
        if ((indices[i - 1] & 0xffff0000U) == (indices[i] & 0xffff0000U)) {
            sa_fast_fix(v, indices, static_cast<int>(i - 1));
        }
    }

    for (uint32_t i = 0; i < n; i++) {
        sa[i] = static_cast<int32_t>(indices[i] & 0xFFFFU);
    }
}

static inline void build_suffix_array_fast_scratch(uint8_t* data, int32_t* sa, uint32_t data_len)
{
    static thread_local uint32_t indices[MAX_LENGTH + 1];
    static thread_local uint32_t tmp_indices[MAX_LENGTH + 1];

    build_suffix_array_fast(data, sa, data_len, indices, tmp_indices);
}

} // namespace astrobwt
} // namespace xmrig

#endif
