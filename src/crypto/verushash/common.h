// Copyright (c) 2014 The Bitcoin Core developers
// Copyright (c) 2016-2023 The Zcash developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://www.opensource.org/licenses/mit-license.php .

#ifndef BITCOIN_CRYPTO_COMMON_H
#define BITCOIN_CRYPTO_COMMON_H

#if defined(HAVE_CONFIG_H)
#include "bitcoin-config.h"
#endif

#include <stdint.h>
#include <assert.h>
#include <string.h>

#if defined(_WIN32)
#include <stdlib.h>
#include <intrin.h>
#ifndef htobe16
#define htobe16(x) _byteswap_ushort((uint16_t)(x))
#endif
#ifndef htole16
#define htole16(x) ((uint16_t)(x))
#endif
#ifndef be16toh
#define be16toh(x) _byteswap_ushort((uint16_t)(x))
#endif
#ifndef le16toh
#define le16toh(x) ((uint16_t)(x))
#endif
#ifndef htobe32
#define htobe32(x) _byteswap_ulong((uint32_t)(x))
#endif
#ifndef htole32
#define htole32(x) ((uint32_t)(x))
#endif
#ifndef be32toh
#define be32toh(x) _byteswap_ulong((uint32_t)(x))
#endif
#ifndef le32toh
#define le32toh(x) ((uint32_t)(x))
#endif
#ifndef htobe64
#define htobe64(x) _byteswap_uint64((uint64_t)(x))
#endif
#ifndef htole64
#define htole64(x) ((uint64_t)(x))
#endif
#ifndef be64toh
#define be64toh(x) _byteswap_uint64((uint64_t)(x))
#endif
#ifndef le64toh
#define le64toh(x) ((uint64_t)(x))
#endif
#elif defined(__APPLE__)
#include <libkern/OSByteOrder.h>
#ifndef htobe16
#define htobe16(x) OSSwapHostToBigInt16((uint16_t)(x))
#endif
#ifndef htole16
#define htole16(x) OSSwapHostToLittleInt16((uint16_t)(x))
#endif
#ifndef be16toh
#define be16toh(x) OSSwapBigToHostInt16((uint16_t)(x))
#endif
#ifndef le16toh
#define le16toh(x) OSSwapLittleToHostInt16((uint16_t)(x))
#endif
#ifndef htobe32
#define htobe32(x) OSSwapHostToBigInt32((uint32_t)(x))
#endif
#ifndef htole32
#define htole32(x) OSSwapHostToLittleInt32((uint32_t)(x))
#endif
#ifndef be32toh
#define be32toh(x) OSSwapBigToHostInt32((uint32_t)(x))
#endif
#ifndef le32toh
#define le32toh(x) OSSwapLittleToHostInt32((uint32_t)(x))
#endif
#ifndef htobe64
#define htobe64(x) OSSwapHostToBigInt64((uint64_t)(x))
#endif
#ifndef htole64
#define htole64(x) OSSwapHostToLittleInt64((uint64_t)(x))
#endif
#ifndef be64toh
#define be64toh(x) OSSwapBigToHostInt64((uint64_t)(x))
#endif
#ifndef le64toh
#define le64toh(x) OSSwapLittleToHostInt64((uint64_t)(x))
#endif
#else
#include <endian.h>
#endif

#if defined(NDEBUG)
// XMRig release builds define NDEBUG; allow building without hard failure.
#endif

uint16_t static inline ReadLE16(const unsigned char* ptr)
{
    uint16_t x;
    memcpy((char*)&x, ptr, 2);
    return le16toh(x);
}

uint32_t static inline ReadLE32(const unsigned char* ptr)
{
    uint32_t x;
    memcpy((char*)&x, ptr, 4);
    return le32toh(x);
}

uint64_t static inline ReadLE64(const unsigned char* ptr)
{
    uint64_t x;
    memcpy((char*)&x, ptr, 8);
    return le64toh(x);
}

void static inline WriteLE16(unsigned char* ptr, uint16_t x)
{
    uint16_t v = htole16(x);
    memcpy(ptr, (char*)&v, 2);
}

void static inline WriteLE32(unsigned char* ptr, uint32_t x)
{
    uint32_t v = htole32(x);
    memcpy(ptr, (char*)&v, 4);
}

void static inline WriteLE64(unsigned char* ptr, uint64_t x)
{
    uint64_t v = htole64(x);
    memcpy(ptr, (char*)&v, 8);
}

uint32_t static inline ReadBE32(const unsigned char* ptr)
{
    uint32_t x;
    memcpy((char*)&x, ptr, 4);
    return be32toh(x);
}

uint64_t static inline ReadBE64(const unsigned char* ptr)
{
    uint64_t x;
    memcpy((char*)&x, ptr, 8);
    return be64toh(x);
}

void static inline WriteBE32(unsigned char* ptr, uint32_t x)
{
    uint32_t v = htobe32(x);
    memcpy(ptr, (char*)&v, 4);
}

void static inline WriteBE64(unsigned char* ptr, uint64_t x)
{
    uint64_t v = htobe64(x);
    memcpy(ptr, (char*)&v, 8);
}

int inline init_and_check_sodium()
{
    // libsodium is not required for the VerusHash PoW path used by xmrig.
    return 0;
}

/** Return the smallest number n such that (x >> n) == 0 (or 64 if the highest bit in x is set. */
uint64_t static inline CountBits(uint64_t x)
{
#ifdef HAVE_DECL___BUILTIN_CLZL
    if (sizeof(unsigned long) >= sizeof(uint64_t)) {
        return x ? 8 * sizeof(unsigned long) - __builtin_clzl(x) : 0;
    }
#endif
#ifdef HAVE_DECL___BUILTIN_CLZLL
    if (sizeof(unsigned long long) >= sizeof(uint64_t)) {
        return x ? 8 * sizeof(unsigned long long) - __builtin_clzll(x) : 0;
    }
#endif
    int ret = 0;
    while (x) {
        x >>= 1;
        ++ret;
    }
    return ret;
}

#endif // BITCOIN_CRYPTO_COMMON_H
