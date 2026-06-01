/* XMRig
 * Copyright (c) 2018-2024 SChernykh   <https://github.com/SChernykh>
 * Copyright (c) 2016-2024 XMRig       <https://github.com/xmrig>, <support@xmrig.com>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef XMRIG_ASTROBWT_HASH_UTILS_H
#define XMRIG_ASTROBWT_HASH_UTILS_H

#include <cstdint>
#include <cstring>

#if defined(__AVX2__) || (defined(_MSC_VER) && defined(__AVX2__))
#   include <immintrin.h>
#   define XMRIG_ASTROBWT_AVX2 1
#endif

namespace xmrig {
namespace astrobwt {


static inline void copy256(void* dst, const void* src)
{
#if defined(XMRIG_ASTROBWT_AVX2)
    auto* d = static_cast<__m256i*>(dst);
    const auto* s = static_cast<const __m256i*>(src);
    d[0] = _mm256_loadu_si256(s + 0);
    d[1] = _mm256_loadu_si256(s + 1);
    d[2] = _mm256_loadu_si256(s + 2);
    d[3] = _mm256_loadu_si256(s + 3);
    d[4] = _mm256_loadu_si256(s + 4);
    d[5] = _mm256_loadu_si256(s + 5);
    d[6] = _mm256_loadu_si256(s + 6);
    d[7] = _mm256_loadu_si256(s + 7);
#else
    std::memcpy(dst, src, 256);
#endif
}


static inline uint8_t rotl8(uint8_t x, int n)
{
    n &= 7;
    return static_cast<uint8_t>((x << n) | (x >> (8 - n)));
}

static inline uint8_t reverse8(uint8_t x)
{
    x = static_cast<uint8_t>(((x & 0xF0) >> 4) | ((x & 0x0F) << 4));
    x = static_cast<uint8_t>(((x & 0xCC) >> 2) | ((x & 0x33) << 2));
    x = static_cast<uint8_t>(((x & 0xAA) >> 1) | ((x & 0x55) << 1));
    return x;
}

static inline uint8_t popcount8(uint8_t x)
{
    x = static_cast<uint8_t>(x - ((x >> 1) & 0x55));
    x = static_cast<uint8_t>((x & 0x33) + ((x >> 2) & 0x33));
    return static_cast<uint8_t>((x + (x >> 4)) & 0x0F);
}


// RC4 cipher matching DERO's implementation (uint32 S-box)
struct RC4 {
    uint32_t s[256];
    uint8_t i, j;

    void init(const uint8_t* key, int keylen)
    {
        for (int k = 0; k < 256; k++) {
            s[k] = static_cast<uint32_t>(k);
        }
        uint8_t jj = 0;
        for (int k = 0; k < 256; k++) {
            jj = static_cast<uint8_t>(jj + static_cast<uint8_t>(s[k]) + key[k % keylen]);
            uint32_t tmp = s[k];
            s[k] = s[jj];
            s[jj] = tmp;
        }
        i = 0;
        j = 0;
    }

    void xor_stream(uint8_t* dst, const uint8_t* src, int len)
    {
        if (len == 256) {
            xor_stream_256(dst, src);
            return;
        }

        for (int k = 0; k < len; k++) {
            i += 1;
            uint32_t x = s[i];
            j = static_cast<uint8_t>(j + static_cast<uint8_t>(x));
            uint32_t y = s[j];
            s[i] = y;
            s[j] = x;
            dst[k] = src[k] ^ static_cast<uint8_t>(s[static_cast<uint8_t>(x + y)]);
        }
    }

private:
    void xor_stream_256(uint8_t* dst, const uint8_t* src)
    {
#define RC4_STEP() \
        do { \
            i += 1; \
            uint32_t x = s[i]; \
            j = static_cast<uint8_t>(j + static_cast<uint8_t>(x)); \
            uint32_t y = s[j]; \
            s[i] = y; \
            s[j] = x; \
            *dst++ = *src++ ^ static_cast<uint8_t>(s[static_cast<uint8_t>(x + y)]); \
        } while (0)

        for (int k = 0; k < 64; k++) {
            RC4_STEP(); RC4_STEP(); RC4_STEP(); RC4_STEP();
        }

#undef RC4_STEP
    }
};


// FNV-1a 64-bit (matching github.com/segmentio/fasthash/fnv1a)
static inline uint64_t fnv1a_hash64(const uint8_t* data, int len)
{
    uint64_t h = 0xcbf29ce484222325ULL;
    for (int i = 0; i < len; i++) {
        h ^= static_cast<uint64_t>(data[i]);
        h *= 0x100000001b3ULL;
    }
    return h;
}


// xxHash64 (matching github.com/cespare/xxhash which implements xxHash64)
namespace xxhash_detail {

static const uint64_t prime1 = 0x9E3779B185EBCA87ULL;
static const uint64_t prime2 = 0xC2B2AE3D27D4EB4FULL;
static const uint64_t prime3 = 0x165667B19E3779F9ULL;
static const uint64_t prime4 = 0x85EBCA77C2B2AE63ULL;
static const uint64_t prime5 = 0x27D4EB2F165667C5ULL;

static inline uint64_t rotl64(uint64_t x, int r) { return (x << r) | (x >> (64 - r)); }

static inline uint64_t read64le(const uint8_t* p)
{
    uint64_t v;
    memcpy(&v, p, 8);
    return v;
}

static inline uint32_t read32le(const uint8_t* p)
{
    uint32_t v;
    memcpy(&v, p, 4);
    return v;
}

static inline uint64_t round(uint64_t acc, uint64_t input)
{
    acc += input * prime2;
    acc = rotl64(acc, 31);
    acc *= prime1;
    return acc;
}

static inline uint64_t mergeRound(uint64_t acc, uint64_t val)
{
    val = round(0, val);
    acc ^= val;
    acc = acc * prime1 + prime4;
    return acc;
}

} // namespace xxhash_detail

static inline uint64_t xxhash64(const uint8_t* data, int len)
{
    using namespace xxhash_detail;
    const uint8_t* p = data;
    const uint8_t* end = data + len;
    uint64_t h;

    if (len >= 32) {
        uint64_t v1 = 0 + prime1 + prime2;
        uint64_t v2 = 0 + prime2;
        uint64_t v3 = 0;
        uint64_t v4 = 0 - prime1;

        while (p + 32 <= end) {
            v1 = round(v1, read64le(p)); p += 8;
            v2 = round(v2, read64le(p)); p += 8;
            v3 = round(v3, read64le(p)); p += 8;
            v4 = round(v4, read64le(p)); p += 8;
        }

        h = rotl64(v1, 1) + rotl64(v2, 7) + rotl64(v3, 12) + rotl64(v4, 18);
        h = mergeRound(h, v1);
        h = mergeRound(h, v2);
        h = mergeRound(h, v3);
        h = mergeRound(h, v4);
    } else {
        h = 0 + prime5;
    }

    h += static_cast<uint64_t>(len);

    while (p + 8 <= end) {
        uint64_t k1 = round(0, read64le(p));
        h ^= k1;
        h = rotl64(h, 27) * prime1 + prime4;
        p += 8;
    }

    while (p + 4 <= end) {
        h ^= static_cast<uint64_t>(read32le(p)) * prime1;
        h = rotl64(h, 23) * prime2 + prime3;
        p += 4;
    }

    while (p < end) {
        h ^= static_cast<uint64_t>(*p) * prime5;
        h = rotl64(h, 11) * prime1;
        p++;
    }

    h ^= h >> 33;
    h *= prime2;
    h ^= h >> 29;
    h *= prime3;
    h ^= h >> 32;

    return h;
}


// SipHash-2-4 (matching github.com/dchest/siphash)
namespace siphash_detail {

static inline uint64_t rotl64(uint64_t x, int r) { return (x << r) | (x >> (64 - r)); }

static inline uint64_t read64le(const uint8_t* p)
{
    uint64_t v;
    memcpy(&v, p, 8);
    return v;
}

} // namespace siphash_detail

static inline uint64_t siphash(uint64_t k0, uint64_t k1, const uint8_t* data, int len)
{
    using namespace siphash_detail;

    uint64_t v0 = k0 ^ 0x736f6d6570736575ULL;
    uint64_t v1 = k1 ^ 0x646f72616e646f6dULL;
    uint64_t v2 = k0 ^ 0x6c7967656e657261ULL;
    uint64_t v3 = k1 ^ 0x7465646279746573ULL;

    const uint8_t* p = data;
    const uint8_t* end = data + (len & ~7);

    while (p < end) {
        uint64_t m = read64le(p);
        v3 ^= m;

        v0 += v1; v1 = rotl64(v1, 13); v1 ^= v0; v0 = rotl64(v0, 32);
        v2 += v3; v3 = rotl64(v3, 16); v3 ^= v2;
        v0 += v3; v3 = rotl64(v3, 21); v3 ^= v0;
        v2 += v1; v1 = rotl64(v1, 17); v1 ^= v2; v2 = rotl64(v2, 32);

        v0 += v1; v1 = rotl64(v1, 13); v1 ^= v0; v0 = rotl64(v0, 32);
        v2 += v3; v3 = rotl64(v3, 16); v3 ^= v2;
        v0 += v3; v3 = rotl64(v3, 21); v3 ^= v0;
        v2 += v1; v1 = rotl64(v1, 17); v1 ^= v2; v2 = rotl64(v2, 32);

        v0 ^= m;
        p += 8;
    }

    uint64_t b = static_cast<uint64_t>(len) << 56;
    const uint8_t* tail = p;
    switch (len & 7) {
    case 7: b |= static_cast<uint64_t>(tail[6]) << 48; /* fallthrough */
    case 6: b |= static_cast<uint64_t>(tail[5]) << 40; /* fallthrough */
    case 5: b |= static_cast<uint64_t>(tail[4]) << 32; /* fallthrough */
    case 4: b |= static_cast<uint64_t>(tail[3]) << 24; /* fallthrough */
    case 3: b |= static_cast<uint64_t>(tail[2]) << 16; /* fallthrough */
    case 2: b |= static_cast<uint64_t>(tail[1]) << 8;  /* fallthrough */
    case 1: b |= static_cast<uint64_t>(tail[0]);        break;
    case 0: break;
    }

    v3 ^= b;

    v0 += v1; v1 = rotl64(v1, 13); v1 ^= v0; v0 = rotl64(v0, 32);
    v2 += v3; v3 = rotl64(v3, 16); v3 ^= v2;
    v0 += v3; v3 = rotl64(v3, 21); v3 ^= v0;
    v2 += v1; v1 = rotl64(v1, 17); v1 ^= v2; v2 = rotl64(v2, 32);

    v0 += v1; v1 = rotl64(v1, 13); v1 ^= v0; v0 = rotl64(v0, 32);
    v2 += v3; v3 = rotl64(v3, 16); v3 ^= v2;
    v0 += v3; v3 = rotl64(v3, 21); v3 ^= v0;
    v2 += v1; v1 = rotl64(v1, 17); v1 ^= v2; v2 = rotl64(v2, 32);

    v0 ^= b;

    v2 ^= 0xff;

    // 4 finalization rounds (SipHash-2-4)
    v0 += v1; v1 = rotl64(v1, 13); v1 ^= v0; v0 = rotl64(v0, 32);
    v2 += v3; v3 = rotl64(v3, 16); v3 ^= v2;
    v0 += v3; v3 = rotl64(v3, 21); v3 ^= v0;
    v2 += v1; v1 = rotl64(v1, 17); v1 ^= v2; v2 = rotl64(v2, 32);

    v0 += v1; v1 = rotl64(v1, 13); v1 ^= v0; v0 = rotl64(v0, 32);
    v2 += v3; v3 = rotl64(v3, 16); v3 ^= v2;
    v0 += v3; v3 = rotl64(v3, 21); v3 ^= v0;
    v2 += v1; v1 = rotl64(v1, 17); v1 ^= v2; v2 = rotl64(v2, 32);

    v0 += v1; v1 = rotl64(v1, 13); v1 ^= v0; v0 = rotl64(v0, 32);
    v2 += v3; v3 = rotl64(v3, 16); v3 ^= v2;
    v0 += v3; v3 = rotl64(v3, 21); v3 ^= v0;
    v2 += v1; v1 = rotl64(v1, 17); v1 ^= v2; v2 = rotl64(v2, 32);

    v0 += v1; v1 = rotl64(v1, 13); v1 ^= v0; v0 = rotl64(v0, 32);
    v2 += v3; v3 = rotl64(v3, 16); v3 ^= v2;
    v0 += v3; v3 = rotl64(v3, 21); v3 ^= v0;
    v2 += v1; v1 = rotl64(v1, 17); v1 ^= v2; v2 = rotl64(v2, 32);

    return v0 ^ v1 ^ v2 ^ v3;
}


} // namespace astrobwt
} // namespace xmrig

#endif
