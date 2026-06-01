// (C) 2018 Michael Toutonghi
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

/*
This provides the PoW hash function for Verus, enabling CPU mining.
*/
#ifndef VERUS_HASH_H_
#define VERUS_HASH_H_

// verbose output when defined
//#define VERUSHASHDEBUG 1

#include <cstdlib>
#include <cstring>
#include <vector>

#include "crypto/verushash/uint256.h"
#include "crypto/verushash/verus_clhash.h"

extern "C" 
{
#include "crypto/verushash/haraka.h"
#include "crypto/verushash/haraka_portable.h"
}

class CVerusHash
{
    public:
        static void Hash(void *result, const void *data, size_t len);
        static void (*haraka512Function)(unsigned char *out, const unsigned char *in);

        static void init();

        CVerusHash() { }

        CVerusHash &Write(const unsigned char *data, size_t len);

        CVerusHash &Reset()
        {
            curBuf = buf1;
            result = buf2;
            curPos = 0;
            std::fill(buf1, buf1 + sizeof(buf1), 0);
            return *this;
        }

        int64_t *ExtraI64Ptr() { return (int64_t *)(curBuf + 32); }
        void ClearExtra()
        {
            if (curPos)
            {
                std::fill(curBuf + 32 + curPos, curBuf + 64, 0);
            }
        }
        void ExtraHash(unsigned char hash[32]) { (*haraka512Function)(hash, curBuf); }

        void Finalize(unsigned char hash[32])
        {
            if (curPos)
            {
                std::fill(curBuf + 32 + curPos, curBuf + 64, 0);
                (*haraka512Function)(hash, curBuf);
            }
            else
                std::memcpy(hash, curBuf, 32);
        }

    private:
        // only buf1, the first source, needs to be zero initialized
        alignas(32) unsigned char buf1[64] = {0}, buf2[64];
        unsigned char *curBuf = buf1, *result = buf2;
        size_t curPos = 0;
};

class CVerusHashV2
{
    public:
        static void Hash(void *result, const void *data, size_t len);
        static void (*haraka512Function)(unsigned char *out, const unsigned char *in);
        static void (*haraka512KeyedFunction)(unsigned char *out, const unsigned char *in, const u128 *rc);
        static void (*haraka256Function)(unsigned char *out, const unsigned char *in);

        static void init();

        verusclhasher vclh;

        CVerusHashV2(int solutionVersion=SOLUTION_VERUSHHASH_V2)
            : vclh(VERUSKEYSIZE, solutionVersion)
            , m_useSimdFinalizeFill(UseSimdFinalizeFill())
            , m_usePackedSuffixWrites(UsePackedSuffixWrites())
            , m_useDirectSv22Path(UseDirectSv22Path())
            , m_useU32SuffixWrites(UseU32SuffixWrites())
            , m_useUnrolledRestore64(UseUnrolledRestore64())
            , m_useSv22Direct(solutionVersion >= SOLUTION_VERUSHHASH_V2_2 && IsCPUVerusOptimized())
            , m_useSv22PortDirect(solutionVersion >= SOLUTION_VERUSHHASH_V2_2 && !IsCPUVerusOptimized())
        {
            // we must have allocated key space, or can't run
            if (!verusclhasher_key.get())
            {
                printf("ERROR: failed to allocate hash buffer - terminating\n");
                assert(false);
            }

            // Pre-dispatch CLHash backend once to keep per-hash hot path branch-free.
            m_preparedClHashFn = vclh.verusclhashfunction;
            if (m_useDirectSv22Path) {
                if (m_useSv22Direct) {
                    m_preparedClHashFn = &verusclhash_sv2_2;
                }
                else if (m_useSv22PortDirect) {
                    m_preparedClHashFn = &verusclhash_sv2_2_port;
                }
            }
            if (!m_preparedClHashFn) {
                // Keep the hot path branch-free even if backend dispatch changes unexpectedly.
                m_preparedClHashFn = vclh.verusclhashfunction;
            }

        }

        CVerusHashV2 &Write(const unsigned char *data, size_t len);

        inline CVerusHashV2 &Reset()
        {
            curBuf = buf1;
            result = buf2;
            curPos = 0;
            std::fill(buf1, buf1 + sizeof(buf1), 0);
            return *this;
        }

        inline int64_t *ExtraI64Ptr() { return (int64_t *)(curBuf + 32); }
        inline void ClearExtra()
        {
            if (curPos)
            {
                std::fill(curBuf + 32 + curPos, curBuf + 64, 0);
            }
        }

        template <typename T>
        inline void FillExtra(const T *_data)
        {
            unsigned char *data = (unsigned char *)_data;
            int pos = curPos;
            int left = 32 - pos;
            do
            {
                int len = left > sizeof(T) ? sizeof(T) : left;
                std::memcpy(curBuf + 32 + pos, data, len);
                pos += len;
                left -= len;
            } while (left > 0);
        }

        inline void ExtraHash(unsigned char hash[32]) { (*haraka512Function)(hash, curBuf); }
        inline void ExtraHashKeyed(unsigned char hash[32], u128 *key) { (*haraka512KeyedFunction)(hash, curBuf, key); }

        void Finalize(unsigned char hash[32])
        {
            if (curPos)
            {
                std::fill(curBuf + 32 + curPos, curBuf + 64, 0);
                (*haraka512Function)(hash, curBuf);
            }
            else
                std::memcpy(hash, curBuf, 32);
        }

        // chains Haraka256 from 32 bytes to fill the key
        static void EnsureCLKeySeed(const unsigned char *seedBytes32)
        {
            unsigned char *key = (unsigned char *)verusclhasher_key.get();
            verusclhash_descr *pdesc = (verusclhash_descr *)verusclhasher_descr.get();
            int size = pdesc->keySizeInBytes;
            int refreshsize = verusclhasher::keymask(size) + 1;
            static thread_local bool s_keySeedInitialized = false;
            // skip keygen if it is the current key
            if (!s_keySeedInitialized || pdesc->seed != *((uint256 *)seedBytes32))
            {
                // generate a new key by chain hashing with Haraka256 from the last curbuf
                int n256blks = size >> 5;
                int nbytesExtra = size & 0x1f;
                unsigned char *pkey = key;
                const unsigned char *psrc = seedBytes32;
                for (int i = 0; i < n256blks; i++)
                {
                    (*haraka256Function)(pkey, psrc);
                    psrc = pkey;
                    pkey += 32;
                }
                if (nbytesExtra)
                {
                    unsigned char buf[32];
                    (*haraka256Function)(buf, psrc);
                    memcpy(pkey, buf, nbytesExtra);
                }
                pdesc->seed = *((uint256 *)seedBytes32);
                s_keySeedInitialized = true;
                memcpy(key + size, key, refreshsize);
                // Initialize move-scratch once so gethashkey() fixup has a valid null terminator.
                memset((unsigned char *)key + (size + refreshsize), 0, size - refreshsize);
            }
        }

        inline uint64_t IntermediateTo128Offset(uint64_t intermediate)
        {
            // the mask is where we wrap
            uint64_t mask = vclh.keyMask >> 4;
            return intermediate & mask;
        }

        void Finalize2b(unsigned char hash[32])
        {
            // fill buffer to the end with the beginning of it to prevent any foreknowledge of
            // bits that may contain zero
            FillExtra((u128 *)curBuf);

#ifdef VERUSHASHDEBUG
            uint256 *bhalf1 = (uint256 *)curBuf;
            uint256 *bhalf2 = bhalf1 + 1;
            printf("Curbuf: %s%s\n", bhalf1->GetHex().c_str(), bhalf2->GetHex().c_str());
#endif

            // ccminer-style: only regenerate key seed when it changes, then apply lightweight fixup.
            EnsureCLKeySeed(curBuf);
            void *keyPtr = nullptr;
            uint64_t intermediate = vclh.hashWithFixup(curBuf, &keyPtr);
            u128 *key = (u128 *)keyPtr;

            // fill buffer to the end with the result
            FillExtra(&intermediate);

#ifdef VERUSHASHDEBUG
            printf("intermediate %lx\n", intermediate);
            printf("Curbuf: %s%s\n", bhalf1->GetHex().c_str(), bhalf2->GetHex().c_str());
            bhalf1 = (uint256 *)key;
            bhalf2 = bhalf1 + ((vclh.keyMask + 1) >> 5);
            printf("   Key: %s%s\n", bhalf1->GetHex().c_str(), bhalf2->GetHex().c_str());
#endif

            // get the final hash with a mutated dynamic key for each hash result
            (*haraka512KeyedFunction)(hash, curBuf, key + IntermediateTo128Offset(intermediate));
        }

        static inline bool UseSimdFinalizeFill()
        {
            static const bool enabled = []() {
                const char *env = std::getenv("XMRIG_VERUS_SIMD_FILL");
                if (!env || !env[0]) {
                    // Latest long-run A/B baseline on this target: disabled unless explicitly enabled.
                    return false;
                }
                return env[0] == '1';
            }();
            return enabled;
        }

        static inline bool UsePackedSuffixWrites()
        {
            static const bool enabled = []() {
                const char *env = std::getenv("XMRIG_VERUS_PACKED_SUFFIX");
                if (!env || !env[0]) {
                    // Latest long-run A/B baseline on this target: enabled unless explicitly disabled.
                    return true;
                }
                return env[0] == '1';
            }();
            return enabled;
        }

        static inline bool UseDirectSv22Path()
        {
            static const bool enabled = []() {
                const char *env = std::getenv("XMRIG_VERUS_DIRECT_SV22");
                return env && env[0] == '1';
            }();
            return enabled;
        }

        static inline bool UseU32SuffixWrites()
        {
            static const bool enabled = []() {
                const char *env = std::getenv("XMRIG_VERUS_U32_SUFFIX");
                if (!env || !env[0]) {
                    // Default on: unaligned u32 stores + packed u32 tail path reduce memcpy/branches on nonce32 mining.
                    return true;
                }
                return env[0] == '1';
            }();
            return enabled;
        }

        static inline bool UseUnrolledRestore64()
        {
            static const bool enabled = []() {
                const char *env = std::getenv("XMRIG_VERUS_RESTORE_UNROLL");
                if (!env || !env[0]) {
                    return true;
                }
                return env[0] == '1';
            }();
            return enabled;
        }

        // Hot-loop variant for prepared jobs where key seed is already prepared once.
        inline void Finalize2bNoSeed(unsigned char hash[32])
        {
            if (curPos == 15) {
                // Hot Verus path: curPos is typically 15 after writing 7-byte suffix.
                // Match ccminer-style fixed-position fill to avoid generic looped copies.
                if (m_useSimdFinalizeFill) {
                    // ccminer-style: shift [0..15] into [47..63] as [0,1..15,0].
                    const __m128i shuf1 = _mm_setr_epi8(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0);
                    const __m128i fill1 = _mm_shuffle_epi8(_mm_load_si128((u128 *)curBuf), shuf1);
                    _mm_store_si128((u128 *)(curBuf + 48), fill1);
                    curBuf[47] = curBuf[0];
                }
                else {
                    std::memcpy(curBuf + 47, curBuf, 16);
                    curBuf[63] = curBuf[0];
                }

                void *keyPtr = nullptr;
                uint64_t intermediate = vclh.hashWithFixup(curBuf, &keyPtr);
                u128 *key = (u128 *)keyPtr;

                if (m_useSimdFinalizeFill) {
                    const __m128i shuf2 = _mm_setr_epi8(1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0);
                    const __m128i fill2 = _mm_shuffle_epi8(_mm_loadl_epi64((u128 *)&intermediate), shuf2);
                    _mm_store_si128((u128 *)(curBuf + 48), fill2);
                    curBuf[47] = *((unsigned char *)&intermediate);
                }
                else {
                    std::memcpy(curBuf + 47, &intermediate, sizeof(intermediate));
                    std::memcpy(curBuf + 55, &intermediate, sizeof(intermediate));
                    curBuf[63] = *((unsigned char *)&intermediate);
                }

                (*haraka512KeyedFunction)(hash, curBuf, key + IntermediateTo128Offset(intermediate));
                return;
            }

            FillExtra((u128 *)curBuf);

            void *keyPtr = nullptr;
            uint64_t intermediate = vclh.hashWithFixup(curBuf, &keyPtr);
            u128 *key = (u128 *)keyPtr;
            FillExtra(&intermediate);

            (*haraka512KeyedFunction)(hash, curBuf, key + IntermediateTo128Offset(intermediate));
        }

        // Hot-loop variant that reuses prepared CLHash pointers from cache.
        inline void Finalize2bNoSeedPrepared(unsigned char hash[32], unsigned char *key, __m128i **pMoveScratch, const uint32_t ofs, const bool restoreMoved)
        {
            if (restoreMoved) {
                Finalize2bNoSeedPreparedRestore(hash, key, pMoveScratch, ofs);
            }
            else {
                Finalize2bNoSeedPreparedNoRestore(hash, key, pMoveScratch);
            }
        }

        inline void Finalize2bNoSeedPreparedNoRestore(unsigned char hash[32], unsigned char *key, __m128i **pMoveScratch)
        {
            Finalize2bNoSeedPreparedImpl<false>(hash, key, pMoveScratch, 0);
        }

        inline void Finalize2bNoSeedPreparedRestore(unsigned char hash[32], unsigned char *key, __m128i **pMoveScratch, const uint32_t ofs)
        {
            Finalize2bNoSeedPreparedImpl<true>(hash, key, pMoveScratch, ofs);
        }






        inline unsigned char *CurBuffer()
        {
            return curBuf;
        }

        inline size_t CurPos() const
        {
            return curPos;
        }

        inline void SetCurPos(size_t inCurPos)
        {
            curPos = inCurPos;
        }

        inline void ExportState(unsigned char outCurBuf[64], unsigned char outResultBuf[64], size_t &outCurPos) const
        {
            std::memcpy(outCurBuf, curBuf, 64);
            std::memcpy(outResultBuf, result, 64);
            outCurPos = curPos;
        }

        inline void ImportStateCurOnly(const unsigned char inCurBuf[64], size_t inCurPos)
        {
            curBuf = buf1;
            result = buf2;
            std::memcpy(curBuf, inCurBuf, 64);
            curPos = inCurPos;
        }

        inline void WriteSuffix7NoCross(const unsigned char suffix[7])
        {
            if (m_usePackedSuffixWrites) {
                uint64_t packed = 0;
                uint64_t preserved = 0;
                std::memcpy(&packed, suffix, 7);
                std::memcpy(&preserved, curBuf + 32 + curPos, sizeof(uint64_t));
                packed = (preserved & 0xFF00000000000000ULL) | (packed & 0x00FFFFFFFFFFFFFFULL);
                std::memcpy(curBuf + 32 + curPos, &packed, sizeof(uint64_t));
            }
            else {
                std::memcpy(curBuf + 32 + curPos, suffix, 7);
            }
            curPos += 7;
        }

        inline void WriteSuffix4PlusTail3NoCross(const unsigned char nonce4[4], const unsigned char tail3[3])
        {
            if (m_usePackedSuffixWrites) {
                uint64_t packed = 0;
                uint64_t preserved = 0;
                std::memcpy(&packed, nonce4, 4);
                std::memcpy(reinterpret_cast<unsigned char *>(&packed) + 4, tail3, 3);
                std::memcpy(&preserved, curBuf + 32 + curPos, sizeof(uint64_t));
                packed = (preserved & 0xFF00000000000000ULL) | (packed & 0x00FFFFFFFFFFFFFFULL);
                std::memcpy(curBuf + 32 + curPos, &packed, sizeof(uint64_t));
            }
            else {
                std::memcpy(curBuf + 32 + curPos, nonce4, 4);
                std::memcpy(curBuf + 32 + curPos + 4, tail3, 3);
            }
            curPos += 7;
        }

        inline void WriteSuffix4PlusTail3NoCrossU32(uint32_t nonce32, const unsigned char tail3[3])
        {
            if (!m_useU32SuffixWrites) {
                WriteSuffix4PlusTail3NoCross(reinterpret_cast<const unsigned char *>(&nonce32), tail3);
                return;
            }

            uint64_t packed = static_cast<uint64_t>(nonce32);
            uint64_t preserved = 0;
            packed |= (static_cast<uint64_t>(tail3[0]) << 32);
            packed |= (static_cast<uint64_t>(tail3[1]) << 40);
            packed |= (static_cast<uint64_t>(tail3[2]) << 48);
            std::memcpy(&preserved, curBuf + 32 + curPos, sizeof(uint64_t));
            packed = (preserved & 0xFF00000000000000ULL) | (packed & 0x00FFFFFFFFFFFFFFULL);
            std::memcpy(curBuf + 32 + curPos, &packed, sizeof(uint64_t));
            curPos += 7;
        }

        inline void WriteSuffix7Cross(const unsigned char suffix[7])
        {
            const int room = 32 - static_cast<int>(curPos);
            std::memcpy(curBuf + 32 + curPos, suffix, room);
            (*haraka512Function)(result, curBuf);
            unsigned char *tmp = curBuf;
            curBuf = result;
            result = tmp;

            const int rem = 7 - room;
            std::memcpy(curBuf + 32, suffix + room, rem);
            curPos = rem;
        }

        inline void OverwriteSuffix7At(size_t pos, const unsigned char suffix[7])
        {
            uint64_t packed = 0;
            uint64_t preserved = 0;
            std::memcpy(&packed, suffix, 7);
            std::memcpy(&preserved, curBuf + 32 + pos, sizeof(uint64_t));
            packed = (preserved & 0xFF00000000000000ULL) | (packed & 0x00FFFFFFFFFFFFFFULL);
            std::memcpy(curBuf + 32 + pos, &packed, sizeof(uint64_t));
        }

        inline void OverwriteSuffix4At(size_t pos, const unsigned char nonce4[4])
        {
            std::memcpy(curBuf + 32 + pos, nonce4, 4);
        }

        inline void OverwriteSuffix4AtU32(size_t pos, uint32_t nonce32)
        {
            if (!m_useU32SuffixWrites) {
                OverwriteSuffix4At(pos, reinterpret_cast<const unsigned char *>(&nonce32));
                return;
            }

#ifdef _MSC_VER
            *reinterpret_cast<uint32_t __unaligned *>(curBuf + 32 + pos) = nonce32;
#else
            std::memcpy(curBuf + 32 + pos, &nonce32, sizeof(uint32_t));
#endif
        }

        inline void ImportState(const unsigned char inCurBuf[64], const unsigned char inResultBuf[64], size_t inCurPos)
        {
            curBuf = buf1;
            result = buf2;
            std::memcpy(curBuf, inCurBuf, 64);
            std::memcpy(result, inResultBuf, 64);
            curPos = inCurPos;
        }

    private:
        template<bool RestoreMoved>
        inline void Finalize2bNoSeedPreparedImpl(unsigned char hash[32], unsigned char *key, __m128i **pMoveScratch, const uint32_t ofs)
        {
            if (RestoreMoved) {
                // Verus CLHash always mutates exactly 32 pairs (64 pointers).
                if (m_useUnrolledRestore64) {
                    for (int i = 0; i < 64; i += 8) {
                        __m128i *p0 = pMoveScratch[i + 0];
                        __m128i *p1 = pMoveScratch[i + 1];
                        __m128i *p2 = pMoveScratch[i + 2];
                        __m128i *p3 = pMoveScratch[i + 3];
                        __m128i *p4 = pMoveScratch[i + 4];
                        __m128i *p5 = pMoveScratch[i + 5];
                        __m128i *p6 = pMoveScratch[i + 6];
                        __m128i *p7 = pMoveScratch[i + 7];

                        *p0 = *(p0 + ofs);
                        *p1 = *(p1 + ofs);
                        *p2 = *(p2 + ofs);
                        *p3 = *(p3 + ofs);
                        *p4 = *(p4 + ofs);
                        *p5 = *(p5 + ofs);
                        *p6 = *(p6 + ofs);
                        *p7 = *(p7 + ofs);
                    }
                }
                else {
                    for (int i = 0; i < 64; ++i) {
                        __m128i *pfixup = pMoveScratch[i];
                        *pfixup = *(pfixup + ofs);
                    }
                }
            }

            if (curPos == 15) {
                if (m_useSimdFinalizeFill) {
                    const __m128i shuf1 = _mm_setr_epi8(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0);
                    const __m128i fill1 = _mm_shuffle_epi8(_mm_load_si128((u128 *)curBuf), shuf1);
                    _mm_store_si128((u128 *)(curBuf + 48), fill1);
                    curBuf[47] = curBuf[0];
                }
                else {
                    std::memcpy(curBuf + 47, curBuf, 16);
                    curBuf[63] = curBuf[0];
                }

                const uint64_t intermediate = CLHashPreparedHot(key, pMoveScratch);
                u128 *key128 = (u128 *)key;

                if (m_useSimdFinalizeFill) {
                    const __m128i shuf2 = _mm_setr_epi8(1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0);
                    const __m128i fill2 = _mm_shuffle_epi8(_mm_loadl_epi64((u128 *)&intermediate), shuf2);
                    _mm_store_si128((u128 *)(curBuf + 48), fill2);
                    curBuf[47] = *((const unsigned char *)&intermediate);
                }
                else {
                    std::memcpy(curBuf + 47, &intermediate, sizeof(intermediate));
                    std::memcpy(curBuf + 55, &intermediate, sizeof(intermediate));
                    curBuf[63] = *((const unsigned char *)&intermediate);
                }

                (*haraka512KeyedFunction)(hash, curBuf, key128 + IntermediateTo128Offset(intermediate));
                return;
            }

            FillExtra((u128 *)curBuf);
            const uint64_t intermediate = CLHashPreparedHot(key, pMoveScratch);
            u128 *key128 = (u128 *)key;
            FillExtra(&intermediate);

            (*haraka512KeyedFunction)(hash, curBuf, key128 + IntermediateTo128Offset(intermediate));
        }

        inline uint64_t CLHashPreparedHot(unsigned char *key, __m128i **pMoveScratch)
        {
            return m_preparedClHashFn(key, curBuf, vclh.keyMask, pMoveScratch);
        }

        // only buf1, the first source, needs to be zero initialized
        alignas(32) unsigned char buf1[64] = {0}, buf2[64];
        unsigned char *curBuf = buf1, *result = buf2;
        size_t curPos = 0;
        bool m_useSimdFinalizeFill = false;
        bool m_usePackedSuffixWrites = true;
        bool m_useDirectSv22Path = false;
        bool m_useU32SuffixWrites = false;
        bool m_useUnrolledRestore64 = true;
        bool m_useSv22Direct = false;
        bool m_useSv22PortDirect = false;
        uint64_t (*m_preparedClHashFn)(void *random, const unsigned char buf[64], uint64_t keyMask, __m128i **pMoveScratch) = nullptr;
};

extern void verus_hash(void *result, const void *data, size_t len);
extern void verus_hash_v2(void *result, const void *data, size_t len);

#endif
