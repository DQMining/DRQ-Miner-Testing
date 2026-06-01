/* XMRig VerusHash integration wrapper
 *
 * Maps xmrig's cn_hash_fun interface to the VerusHash v2 reference
 * implementation (verus_hash_v2).
 */

#include "crypto/verushash/VerusHash_xmrig.h"
#include <cstdlib>
#include <cstring>

namespace xmrig {
namespace verushash {

static bool s_initialized = false;

namespace {

#if defined(_MSC_VER)
#   define XMRIG_VERUS_FORCE_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#   define XMRIG_VERUS_FORCE_INLINE __attribute__((always_inline)) inline
#else
#   define XMRIG_VERUS_FORCE_INLINE inline
#endif

static inline bool useRollingNonce32Path()
{
    static const bool enabled = []() {
        const char *env = std::getenv("XMRIG_VERUS_ROLLING_NONCE32");
        if (!env || !env[0]) {
            return true;
        }

        return env[0] != '0';
    }();
    return enabled;
}

static inline uint64_t loadU64(const uint8_t *p)
{
    uint64_t v = 0;
    std::memcpy(&v, p, sizeof(v));
    return v;
}

struct FastJobCache
{
    enum class HotWriteMode : uint8_t {
        RollingFast = 0,
        DynamicNoCrossBytes,
        DynamicCrossBytes,
        Suffix7NoCrossBytes,
        Suffix7CrossBytes,
        SlowTail
    };
    CVerusHashV2 prepStateV2{SOLUTION_VERUSHHASH_V2};
    CVerusHashV2 prepStateV21{SOLUTION_VERUSHHASH_V2_1};
    CVerusHashV2 prepStateV22{SOLUTION_VERUSHHASH_V2_2};
    CVerusHashV2 workStateV2{SOLUTION_VERUSHHASH_V2};
    CVerusHashV2 workStateV21{SOLUTION_VERUSHHASH_V2_1};
    CVerusHashV2 workStateV22{SOLUTION_VERUSHHASH_V2_2};
    uint64_t token = 0;
    size_t size = 0;
    size_t nonceOffset = 0;
    int solutionVersion = SOLUTION_VERUSHHASH_V2;
    uint64_t headWord = 0;
    uint64_t preNonceWord = 0;
    unsigned char snapshotCur[64]{};
    unsigned char snapshotResult[64]{};
    size_t snapshotCurPos = 0;
    size_t snapshotCurPosWithSuffix = 0;
    bool suffix7NoCross = false;
    bool dynamicNonce4 = false;
    unsigned char tail3[3]{};
    unsigned char *clKey = nullptr;
    __m128i **clMoveScratch = nullptr;
    uint32_t clFixupOfs = 0;
    CVerusHashV2 *work = nullptr;
    bool canFast = false;
    bool usePreparedFinalize = false;
    bool rollingFastPath = false;
    bool rollingReady = false;
    HotWriteMode hotWriteMode = HotWriteMode::SlowTail;
    bool clMoveScratchReady = false;
    bool ready = false;
};

static inline FastJobCache &fastCache()
{
    thread_local FastJobCache cache;
    return cache;
}

static inline CVerusHashV2 *selectPrepState(FastJobCache &cache, int solutionVersion)
{
    if (solutionVersion >= SOLUTION_VERUSHHASH_V2_2) {
        return &cache.prepStateV22;
    }
    if (solutionVersion >= SOLUTION_VERUSHHASH_V2_1) {
        return &cache.prepStateV21;
    }

    return &cache.prepStateV2;
}

static inline CVerusHashV2 *selectWorkState(FastJobCache &cache, int solutionVersion)
{
    if (solutionVersion >= SOLUTION_VERUSHHASH_V2_2) {
        return &cache.workStateV22;
    }
    if (solutionVersion >= SOLUTION_VERUSHHASH_V2_1) {
        return &cache.workStateV21;
    }

    return &cache.workStateV2;
}

static XMRIG_VERUS_FORCE_INLINE CVerusHashV2 *workStateHot(FastJobCache &cache)
{
    CVerusHashV2 *work = cache.work;
    if (!work) {
        work = selectWorkState(cache, cache.solutionVersion);
        cache.work = work;
    }

    return work;
}

static inline void prepareCache(const uint8_t *input, size_t size, uint64_t cacheToken, FastJobCache &cache)
{
    int solutionVersion = SOLUTION_VERUSHHASH_V2;
    if (size > 144 && input[140] == 0xfd) {
        solutionVersion = input[143];
    }

    const bool canFast = (size > 11);
    const size_t nonceOffset = canFast ? (size - 7) : 0;
    const uint64_t headWord = (size >= sizeof(uint64_t)) ? loadU64(input) : 0;
    const uint64_t preNonceWord = (canFast && nonceOffset >= sizeof(uint64_t)) ? loadU64(input + nonceOffset - sizeof(uint64_t)) : 0;
    const bool cacheMatch = cache.ready &&
                            cache.token == cacheToken &&
                            cache.size == size &&
                            cache.nonceOffset == nonceOffset &&
                            cache.solutionVersion == solutionVersion &&
                            cache.headWord == headWord &&
                            cache.preNonceWord == preNonceWord;
    if (cacheMatch) {
        return;
    }

    CVerusHashV2 *prep = selectPrepState(cache, solutionVersion);
    prep->Reset();
    if (canFast) {
        prep->Write(input, nonceOffset);
    }
    else {
        prep->Write(input, size);
    }
    prep->ExportState(cache.snapshotCur, cache.snapshotResult, cache.snapshotCurPos);
    // ccminer-like: prepare key seed once from prepared half-state.
    CVerusHashV2::EnsureCLKeySeed(cache.snapshotCur);
    cache.clKey = (unsigned char *)verusclhasher_key.get();
    verusclhash_descr *desc = (verusclhash_descr *)verusclhasher_descr.get();
    if (cache.clKey && desc) {
        cache.clFixupOfs = desc->keySizeInBytes >> 4;
        cache.clMoveScratch = prep->vclh.getpmovescratch(cache.clKey + desc->keySizeInBytes);
    }
    else {
        cache.clMoveScratch = nullptr;
        cache.clFixupOfs = 0;
    }
    cache.token = cacheToken;
    cache.size = size;
    cache.nonceOffset = nonceOffset;
    cache.solutionVersion = solutionVersion;
    cache.work = selectWorkState(cache, solutionVersion);
    cache.canFast = canFast;
    cache.headWord = headWord;
    cache.preNonceWord = preNonceWord;
    cache.snapshotCurPosWithSuffix = cache.snapshotCurPos + 7;
    cache.suffix7NoCross = canFast && ((size - nonceOffset) == 7) && (cache.snapshotCurPos <= 25);
    cache.dynamicNonce4 = canFast && ((size - nonceOffset) == 7);
    if (cache.dynamicNonce4) {
        std::memcpy(cache.tail3, input + nonceOffset + 4, sizeof(cache.tail3));
    }
    cache.rollingFastPath = cache.suffix7NoCross && (cache.snapshotCurPosWithSuffix <= 32);
    if (cache.rollingFastPath) {
        cache.hotWriteMode = FastJobCache::HotWriteMode::RollingFast;
    }
    else if (cache.dynamicNonce4) {
        cache.hotWriteMode = cache.suffix7NoCross ? FastJobCache::HotWriteMode::DynamicNoCrossBytes : FastJobCache::HotWriteMode::DynamicCrossBytes;
    }
    else if (cache.canFast) {
        cache.hotWriteMode = cache.suffix7NoCross ? FastJobCache::HotWriteMode::Suffix7NoCrossBytes : FastJobCache::HotWriteMode::Suffix7CrossBytes;
    }
    else {
        cache.hotWriteMode = FastJobCache::HotWriteMode::SlowTail;
    }
    cache.rollingReady = false;
    cache.clMoveScratchReady = false;
    cache.usePreparedFinalize = (cache.clKey && cache.clMoveScratch);
    cache.ready = true;
}

static XMRIG_VERUS_FORCE_INLINE void finalizePreparedHot(FastJobCache &cache, CVerusHashV2 *work, uint8_t *output)
{
    if (cache.usePreparedFinalize) {
        if (cache.clMoveScratchReady) {
            work->Finalize2bNoSeedPreparedRestore(output, cache.clKey, cache.clMoveScratch, cache.clFixupOfs);
        }
        else {
            work->Finalize2bNoSeedPreparedNoRestore(output, cache.clKey, cache.clMoveScratch);
            cache.clMoveScratchReady = true;
        }
    }
    else {
        work->Finalize2bNoSeed(output);
    }
}

} // namespace

void init()
{
    if (!s_initialized) {
        CVerusHashV2::init();
        s_initialized = true;
    }
}


void prepare_fast_job(const uint8_t *input, size_t size, uint64_t cacheToken)
{
    if (!s_initialized) {
        init();
    }

    FastJobCache &cache = fastCache();
    prepareCache(input, size, cacheToken, cache);
}

bool prepared_hot_is_rolling_nonce32()
{
    FastJobCache &cache = fastCache();
    return cache.ready && cache.rollingFastPath && useRollingNonce32Path();
}

bool prepared_hot_is_dynamic_nonce4()
{
    FastJobCache &cache = fastCache();
    return cache.ready && cache.dynamicNonce4;
}

bool prepared_hot_is_suffix7_no_cross()
{
    FastJobCache &cache = fastCache();
    return cache.ready && cache.suffix7NoCross;
}

void single_hash_fast_prepared(const uint8_t *input, size_t size, uint8_t *output, uint64_t cacheToken)
{
    if (!s_initialized) {
        init();
    }

    FastJobCache &cache = fastCache();
    if (!cache.ready || cache.token != cacheToken || cache.size != size) {
        prepareCache(input, size, cacheToken, cache);
    }

    if (!cache.ready || cache.nonceOffset > size) {
        return single_hash_fast(input, size, output, cacheToken);
    }

    CVerusHashV2 *work = cache.work ? cache.work : selectWorkState(cache, cache.solutionVersion);
    if (cache.canFast) {
        work->ImportStateCurOnly(cache.snapshotCur, cache.snapshotCurPos);
        if (cache.dynamicNonce4) {
            if (cache.suffix7NoCross) {
                work->WriteSuffix4PlusTail3NoCross(input + cache.nonceOffset, cache.tail3);
            }
            else {
                unsigned char nonceTail[7];
                std::memcpy(nonceTail, input + cache.nonceOffset, 4);
                std::memcpy(nonceTail + 4, cache.tail3, 3);
                work->WriteSuffix7Cross(nonceTail);
            }
        }
        else if (cache.suffix7NoCross) {
            work->WriteSuffix7NoCross(input + cache.nonceOffset);
        }
        else {
            work->WriteSuffix7Cross(input + cache.nonceOffset);
        }
    } else {
        work->ImportState(cache.snapshotCur, cache.snapshotResult, cache.snapshotCurPos);
        work->Write(input + cache.nonceOffset, size - cache.nonceOffset);
    }
    finalizePreparedHot(cache, work, output);
}

void single_hash_fast_prepared_hot(const uint8_t *input, uint8_t *output)
{
    FastJobCache &cache = fastCache();
    CVerusHashV2 *work = workStateHot(cache);

    switch (cache.hotWriteMode) {
    case FastJobCache::HotWriteMode::RollingFast:
        if (!cache.rollingReady) {
            work->ImportStateCurOnly(cache.snapshotCur, cache.snapshotCurPos);
            std::memcpy(work->CurBuffer() + 32 + cache.snapshotCurPos + 4, cache.tail3, sizeof(cache.tail3));
            work->SetCurPos(cache.snapshotCurPosWithSuffix);
            cache.rollingReady = true;
        }
        work->OverwriteSuffix4At(cache.snapshotCurPos, input + cache.nonceOffset);
        break;

    case FastJobCache::HotWriteMode::DynamicNoCrossBytes:
        work->ImportStateCurOnly(cache.snapshotCur, cache.snapshotCurPos);
        work->WriteSuffix4PlusTail3NoCross(input + cache.nonceOffset, cache.tail3);
        break;

    case FastJobCache::HotWriteMode::DynamicCrossBytes: {
        work->ImportStateCurOnly(cache.snapshotCur, cache.snapshotCurPos);
        unsigned char nonceTail[7];
        std::memcpy(nonceTail, input + cache.nonceOffset, 4);
        std::memcpy(nonceTail + 4, cache.tail3, 3);
        work->WriteSuffix7Cross(nonceTail);
        break;
    }

    case FastJobCache::HotWriteMode::Suffix7NoCrossBytes:
        work->ImportStateCurOnly(cache.snapshotCur, cache.snapshotCurPos);
        work->WriteSuffix7NoCross(input + cache.nonceOffset);
        break;

    case FastJobCache::HotWriteMode::Suffix7CrossBytes:
        work->ImportStateCurOnly(cache.snapshotCur, cache.snapshotCurPos);
        work->WriteSuffix7Cross(input + cache.nonceOffset);
        break;

    case FastJobCache::HotWriteMode::SlowTail:
        work->ImportState(cache.snapshotCur, cache.snapshotResult, cache.snapshotCurPos);
        work->Write(input + cache.nonceOffset, cache.size - cache.nonceOffset);
        break;
    }

    finalizePreparedHot(cache, work, output);
}

void single_hash_fast_prepared_hot_nonce32(const uint8_t *input, uint32_t nonce32, uint8_t *output)
{
    FastJobCache &cache = fastCache();
    CVerusHashV2 *work = workStateHot(cache);

    if (cache.rollingFastPath) {
        if (!cache.rollingReady) {
            work->ImportStateCurOnly(cache.snapshotCur, cache.snapshotCurPos);
            std::memcpy(work->CurBuffer() + 32 + cache.snapshotCurPos + 4, cache.tail3, sizeof(cache.tail3));
            work->SetCurPos(cache.snapshotCurPosWithSuffix);
            cache.rollingReady = true;
        }
        work->OverwriteSuffix4AtU32(cache.snapshotCurPos, nonce32);
    }
    else {
        if (cache.dynamicNonce4) {
            work->ImportStateCurOnly(cache.snapshotCur, cache.snapshotCurPos);
            if (cache.suffix7NoCross) {
                work->WriteSuffix4PlusTail3NoCrossU32(nonce32, cache.tail3);
            }
            else {
                unsigned char nonceTail[7];
                std::memcpy(nonceTail, &nonce32, 4);
                std::memcpy(nonceTail + 4, cache.tail3, 3);
                work->WriteSuffix7Cross(nonceTail);
            }
        }
        else if (cache.canFast) {
            work->ImportStateCurOnly(cache.snapshotCur, cache.snapshotCurPos);
            if (cache.suffix7NoCross) {
                work->WriteSuffix7NoCross(input + cache.nonceOffset);
            }
            else {
                work->WriteSuffix7Cross(input + cache.nonceOffset);
            }
        }
        else {
            work->ImportState(cache.snapshotCur, cache.snapshotResult, cache.snapshotCurPos);
            work->Write(input + cache.nonceOffset, cache.size - cache.nonceOffset);
        }
    }

    finalizePreparedHot(cache, work, output);
}

void single_hash_fast_prepared_hot_nonce32_rolling(const uint8_t *input, uint32_t nonce32, uint8_t *output)
{
    FastJobCache &cache = fastCache();
    CVerusHashV2 *work = workStateHot(cache);

    if (!cache.rollingFastPath) {
        return single_hash_fast_prepared_hot_nonce32(input, nonce32, output);
    }

    if (!cache.rollingReady) {
        work->ImportStateCurOnly(cache.snapshotCur, cache.snapshotCurPos);
        std::memcpy(work->CurBuffer() + 32 + cache.snapshotCurPos + 4, cache.tail3, sizeof(cache.tail3));
        work->SetCurPos(cache.snapshotCurPosWithSuffix);
        cache.rollingReady = true;
    }

    work->OverwriteSuffix4AtU32(cache.snapshotCurPos, nonce32);

    finalizePreparedHot(cache, work, output);
}

void single_hash_fast_prepared_hot_nonce32_dynamic_no_cross(uint32_t nonce32, uint8_t *output)
{
    FastJobCache &cache = fastCache();
    CVerusHashV2 *work = workStateHot(cache);

    work->ImportStateCurOnly(cache.snapshotCur, cache.snapshotCurPos);
    work->WriteSuffix4PlusTail3NoCrossU32(nonce32, cache.tail3);

    finalizePreparedHot(cache, work, output);
}

void single_hash_fast_prepared_hot_nonce32_dynamic_cross(uint32_t nonce32, uint8_t *output)
{
    FastJobCache &cache = fastCache();
    CVerusHashV2 *work = workStateHot(cache);

    work->ImportStateCurOnly(cache.snapshotCur, cache.snapshotCurPos);
    unsigned char nonceTail[7];
    std::memcpy(nonceTail, &nonce32, 4);
    std::memcpy(nonceTail + 4, cache.tail3, 3);
    work->WriteSuffix7Cross(nonceTail);

    finalizePreparedHot(cache, work, output);
}

void single_hash_fast_prepared_unchecked(const uint8_t *input, size_t size, uint8_t *output)
{
    if (!s_initialized) {
        init();
    }

    FastJobCache &cache = fastCache();
    if (!cache.ready || cache.size != size || cache.nonceOffset > size) {
        return single_hash_fast(input, size, output, 0);
    }

    single_hash_fast_prepared_hot(input, output);
}


void single_hash_fast(const uint8_t *input, size_t size, uint8_t *output, uint64_t cacheToken)
{
    prepare_fast_job(input, size, cacheToken);
    single_hash_fast_prepared(input, size, output, cacheToken);
}


template<>
void single_hash<Algorithm::VERUSHASH>(const uint8_t *input, size_t size, uint8_t *output, cryptonight_ctx **, uint64_t)
{
    single_hash_fast(input, size, output, 0);
}


} // namespace verushash
} // namespace xmrig

