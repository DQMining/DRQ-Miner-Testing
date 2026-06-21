/* XMRig
 * Copyright (c) 2018-2021 SChernykh   <https://github.com/SChernykh>
 * Copyright (c) 2016-2021 XMRig       <https://github.com/xmrig>, <support@xmrig.com>
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

#include <thread>
#include <mutex>
#include <cstdlib>


#include "base/io/log/Log.h"
#include "backend/cpu/Cpu.h"
#include "backend/cpu/CpuWorker.h"
#include "base/tools/Alignment.h"
#include "base/tools/Chrono.h"
#include "core/config/Config.h"
#include "core/Miner.h"
#include "crypto/cn/CnCtx.h"
#include "crypto/cn/CryptoNight_test.h"
#include "crypto/cn/CryptoNight.h"
#include "crypto/common/Nonce.h"
#include "crypto/common/VirtualMemory.h"
#include "crypto/rx/Rx.h"
#include "crypto/rx/RxCache.h"
#include "crypto/rx/RxDataset.h"
#include "crypto/rx/RxVm.h"
#include "crypto/ghostrider/ghostrider.h"
#include "crypto/verushash/VerusHash_xmrig.h"
#include "net/JobResults.h"


#ifdef XMRIG_ALGO_RANDOMX
#   include "crypto/randomx/randomx.h"
#endif


#ifdef XMRIG_FEATURE_BENCHMARK
#   include "backend/common/benchmark/BenchState.h"
#endif

#ifdef XMRIG_ALGO_ASTROBWT
#   include "crypto/astrobwt/AstroBWT.h"
#endif

#ifdef XMRIG_ALGO_NM
#   include "crypto/nm/nm_fast.h"
#   include "crypto/nm/NmShared.h"
#endif


namespace xmrig {

static constexpr uint32_t kReserveCount = 16384;

#ifdef XMRIG_ALGO_ASTROBWT
static constexpr int kAstroBatch = 512;
#endif

#ifdef XMRIG_ALGO_VERUSHASH
#if defined(__GNUC__) || defined(__clang__)
#   define XMRIG_LIKELY(x) __builtin_expect(!!(x), 1)
#   define XMRIG_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#   define XMRIG_LIKELY(x) (x)
#   define XMRIG_UNLIKELY(x) (x)
#endif

static inline int verusBatchSize()
{
    static const int batch = []() {
        // Larger batches amortize per-batch setup vs SRB-style inner loops (tunable via XMRIG_VERUS_BATCH).
        constexpr int kDefaultBatch = 54;
        const char *env = std::getenv("XMRIG_VERUS_BATCH");
        if (!env || !env[0]) {
            return kDefaultBatch;
        }

        int parsed = std::atoi(env);
        if (parsed < 1 || parsed > 256) {
            return kDefaultBatch;
        }

        return parsed;
    }();

    return batch;
}

static inline uint32_t verusReserveSize()
{
    static const uint32_t reserve = []() -> uint32_t {
        // 64Ki power-of-two reserve: simpler mod in allocator hot paths vs 98304 (override with XMRIG_VERUS_RESERVE).
        constexpr uint32_t kDefaultReserve = 65536U;
        const char *env = std::getenv("XMRIG_VERUS_RESERVE");
        if (!env || !env[0]) {
            return kDefaultReserve;
        }

        int parsed = std::atoi(env);
        if (parsed < 1024 || parsed > 262144) {
            return kDefaultReserve;
        }

        const uint32_t value = static_cast<uint32_t>(parsed);
        const char *allowAny = std::getenv("XMRIG_VERUS_RESERVE_ANY");
        const bool allowNonPowerOfTwo = allowAny && allowAny[0] == '1';
        if (!allowNonPowerOfTwo && (value & (value - 1U)) != 0U) {
            return kDefaultReserve;
        }

        return value;
    }();

    return reserve;
}

static inline bool verusUseDirectNonce32()
{
    static const bool enabled = []() {
        const char *env = std::getenv("XMRIG_VERUS_DIRECT_NONCE32");
        if (!env || !env[0]) {
            // Latest 24h runs favored prepared-hot control over direct nonce32.
            return false;
        }

        return env[0] == '1';
    }();

    return enabled;
}

static inline bool verusForceNonce32Prepared()
{
    static const bool enabled = []() {
        const char *env = std::getenv("XMRIG_VERUS_FORCE_NONCE32");
        return env && env[0] == '1';
    }();

    return enabled;
}
#endif

#ifdef _MSC_VER
static __forceinline uint64_t loadU64Hot(const uint8_t *p)
{
    return *reinterpret_cast<const uint64_t __unaligned *>(p);
}

static __forceinline uint32_t loadU32Hot(const uint32_t *p)
{
    return *reinterpret_cast<const uint32_t __unaligned *>(p);
}

static __forceinline void storeU32Hot(uint32_t *p, uint32_t v)
{
    *reinterpret_cast<uint32_t __unaligned *>(p) = v;
}
#else
static inline __attribute__((always_inline)) uint64_t loadU64Hot(const uint8_t *p)
{
    return readUnaligned(reinterpret_cast<const uint64_t *>(p));
}

static inline __attribute__((always_inline)) uint32_t loadU32Hot(const uint32_t *p)
{
    return readUnaligned(p);
}

static inline __attribute__((always_inline)) void storeU32Hot(uint32_t *p, uint32_t v)
{
    writeUnaligned(p, v);
}
#endif

#ifdef XMRIG_ALGO_CN_HEAVY
static std::mutex cn_heavyZen3MemoryMutex;
VirtualMemory* cn_heavyZen3Memory = nullptr;
#endif

} // namespace xmrig



template<size_t N>
xmrig::CpuWorker<N>::CpuWorker(size_t id, const CpuLaunchData &data) :
    Worker(id, data.affinity, data.priority),
    m_algorithm(data.algorithm),
    m_assembly(data.assembly),
    m_hwAES(data.hwAES),
    m_yield(data.yield),
    m_av(data.av()),
    m_miner(data.miner),
    m_threads(data.threads),
    m_ctx()
{
#   ifdef XMRIG_ALGO_NM
    m_nmHugePages = data.hugePages;
#   endif

#   ifdef XMRIG_ALGO_CN_HEAVY
    // cn-heavy optimization for Zen3 CPUs
    const auto arch = Cpu::info()->arch();
    const uint32_t model = Cpu::info()->model();
    const bool is_vermeer = (arch == ICpuInfo::ARCH_ZEN3) && (model == 0x21);
    const bool is_raphael = (arch == ICpuInfo::ARCH_ZEN4) && (model == 0x61);
    if ((N == 1) && (m_av == CnHash::AV_SINGLE) && (m_algorithm.family() == Algorithm::CN_HEAVY) && (m_assembly != Assembly::NONE) && (is_vermeer || is_raphael)) {
        std::lock_guard<std::mutex> lock(cn_heavyZen3MemoryMutex);
        if (!cn_heavyZen3Memory) {
            // Round up number of threads to the multiple of 8
            const size_t num_threads = ((m_threads + 7) / 8) * 8;
            cn_heavyZen3Memory = new VirtualMemory(m_algorithm.l3() * num_threads, data.hugePages, false, false, node(), VirtualMemory::kDefaultHugePageSize);
        }
        m_memory = cn_heavyZen3Memory;
    }
    else
#   endif
    {
        m_memory = new VirtualMemory(m_algorithm.l3() * N, data.hugePages, false, true, node(), VirtualMemory::kDefaultHugePageSize);
    }

#   ifdef XMRIG_ALGO_GHOSTRIDER
    m_ghHelper = ghostrider::create_helper_thread(affinity(), data.priority, data.affinities);
#   endif
}


template<size_t N>
xmrig::CpuWorker<N>::~CpuWorker()
{
#   ifdef XMRIG_ALGO_RANDOMX
    RxVm::destroy(m_vm);
#   endif

#   ifdef XMRIG_ALGO_NM
    if (m_nmInit) {
        free(m_nmLane.prog);
        free(m_nmLane.taken);
    }
#   endif

    CnCtx::release(m_ctx, N);

#   ifdef XMRIG_ALGO_CN_HEAVY
    if (m_memory != cn_heavyZen3Memory)
#   endif
    {
        delete m_memory;
    }

#   ifdef XMRIG_ALGO_GHOSTRIDER
    ghostrider::destroy_helper_thread(m_ghHelper);
#   endif
}


#ifdef XMRIG_ALGO_RANDOMX
template<size_t N>
void xmrig::CpuWorker<N>::allocateRandomX_VM()
{
    RxDataset *dataset = Rx::dataset(m_job.currentJob(), node());

    while (dataset == nullptr) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));

        if (Nonce::sequence(Nonce::CPU) == 0) {
            return;
        }

        dataset = Rx::dataset(m_job.currentJob(), node());
    }

    if (!m_vm) {
        // Try to allocate scratchpad from dataset's 1 GB huge pages, if normal huge pages are not available
        uint8_t* scratchpad = m_memory->isHugePages() ? m_memory->scratchpad() : dataset->tryAllocateScrathpad();
        m_vm = RxVm::create(dataset, scratchpad ? scratchpad : m_memory->scratchpad(), !m_hwAES, m_assembly, node());
    }
    else if (!dataset->get() && (m_job.currentJob().seed() != m_seed)) {
        // Update RandomX light VM with the new seed
        randomx_vm_set_cache(m_vm, dataset->cache()->get());
    }
    m_seed = m_job.currentJob().seed();
}
#endif


#ifdef XMRIG_ALGO_NM
template<size_t N>
void xmrig::CpuWorker<N>::allocateNm()
{
    if (!m_nmInit) {
        m_nmLane.prog = static_cast<nm_instr *>(malloc(static_cast<size_t>(NM_PROG_MAX) * sizeof(nm_instr)));
        m_nmLane.taken = static_cast<uint8_t *>(malloc(NM_PROG_MAX));
        if (!m_nmLane.prog || !m_nmLane.taken) {
            free(m_nmLane.prog);
            free(m_nmLane.taken);
            m_nmLane.prog = nullptr;
            m_nmLane.taken = nullptr;
            return;
        }
        nm_fast_lane_attach(&m_nmLane, m_memory->scratchpad());
        m_nmInit = true;
    }

    const Job &job = m_job.currentJob();
    const Buffer &seed = job.seed();
    if (seed.size() != 32) {
        return;
    }

    if (!m_nmSeed.empty() && m_nmSeed.size() == seed.size() && memcmp(m_nmSeed.data(), seed.data(), seed.size()) == 0) {
        return;
    }

    m_nmEpoch = NmShared::epoch(seed.data(), node(), m_nmHugePages);
    if (!m_nmEpoch) {
        return;
    }

    static bool fillLogged = false;
    if (!fillLogged) {
        fillLogged = true;
        LOG_INFO("nm scratch fill: %s", nm_fast_fill_name());
    }

    m_nmSeed = seed;
}
#endif


template<size_t N>
bool xmrig::CpuWorker<N>::selfTest()
{
#   ifdef XMRIG_ALGO_NM
    if (m_algorithm.family() == Algorithm::NM) {
        return N == 1;
    }
#   endif

#   ifdef XMRIG_ALGO_RANDOMX
    if (m_algorithm.family() == Algorithm::RANDOM_X) {
        return N == 1;
    }
#   endif

    allocateCnCtx();

#   ifdef XMRIG_ALGO_GHOSTRIDER
    if (m_algorithm.family() == Algorithm::GHOSTRIDER) {
        return (N == 8) && verify(Algorithm::GHOSTRIDER_RTM, test_output_gr);
    }
#   endif

    if (m_algorithm.family() == Algorithm::CN) {
        const bool rc = verify(Algorithm::CN_0,      test_output_v0)   &&
                        verify(Algorithm::CN_1,      test_output_v1)   &&
                        verify(Algorithm::CN_2,      test_output_v2)   &&
                        verify(Algorithm::CN_FAST,   test_output_msr)  &&
                        verify(Algorithm::CN_XAO,    test_output_xao)  &&
                        verify(Algorithm::CN_RTO,    test_output_rto)  &&
                        verify(Algorithm::CN_HALF,   test_output_half) &&
                        verify2(Algorithm::CN_R,     test_output_r)    &&
                        verify(Algorithm::CN_RWZ,    test_output_rwz)  &&
                        verify(Algorithm::CN_ZLS,    test_output_zls)  &&
                        verify(Algorithm::CN_CCX,    test_output_ccx)  &&
                        verify(Algorithm::CN_DOUBLE, test_output_double);

        return rc;
    }

#   ifdef XMRIG_ALGO_CN_LITE
    if (m_algorithm.family() == Algorithm::CN_LITE) {
        return verify(Algorithm::CN_LITE_0,    test_output_v0_lite) &&
               verify(Algorithm::CN_LITE_1,    test_output_v1_lite);
    }
#   endif

#   ifdef XMRIG_ALGO_CN_HEAVY
    if (m_algorithm.family() == Algorithm::CN_HEAVY) {
        return verify(Algorithm::CN_HEAVY_0,    test_output_v0_heavy)  &&
               verify(Algorithm::CN_HEAVY_XHV,  test_output_xhv_heavy) &&
               verify(Algorithm::CN_HEAVY_TUBE, test_output_tube_heavy);
    }
#   endif

#   ifdef XMRIG_ALGO_CN_PICO
    if (m_algorithm.family() == Algorithm::CN_PICO) {
        return verify(Algorithm::CN_PICO_0, test_output_pico_trtl) &&
               verify(Algorithm::CN_PICO_TLO, test_output_pico_tlo);
    }
#   endif

#   ifdef XMRIG_ALGO_CN_FEMTO
    if (m_algorithm.family() == Algorithm::CN_FEMTO) {
        return verify(Algorithm::CN_UPX2, test_output_femto_upx2);
    }
#   endif

#   ifdef XMRIG_ALGO_ARGON2
    if (m_algorithm.family() == Algorithm::ARGON2) {
        return verify(Algorithm::AR2_CHUKWA, argon2_chukwa_test_out) &&
               verify(Algorithm::AR2_CHUKWA_V2, argon2_chukwa_v2_test_out) &&
               verify(Algorithm::AR2_WRKZ, argon2_wrkz_test_out);
    }
#   endif

#   ifdef XMRIG_ALGO_VERUSHASH
    if (m_algorithm.family() == Algorithm::VERUSHASH_FAMILY) {
        return N == 1;
    }
#   endif

#   ifdef XMRIG_ALGO_ASTROBWT
    if (m_algorithm.family() == Algorithm::ASTROBWT) {
        return N == 1;
    }
#   endif

    return false;
}


template<size_t N>
void xmrig::CpuWorker<N>::hashrateData(uint64_t &hashCount, uint64_t &, uint64_t &rawHashes) const
{
    hashCount = m_count;
    rawHashes = m_count;
}


template<size_t N>
void xmrig::CpuWorker<N>::start()
{
    while (Nonce::sequence(Nonce::CPU) > 0) {
        if (Nonce::isPaused()) {
            do {
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
            }
            while (Nonce::isPaused() && Nonce::sequence(Nonce::CPU) > 0);

            if (Nonce::sequence(Nonce::CPU) == 0) {
                break;
            }

            consumeJob();
        }

#       ifdef XMRIG_ALGO_RANDOMX
        bool first = true;
        alignas(16) uint64_t tempHash[8] = {};
#       endif
#       ifdef XMRIG_ALGO_VERUSHASH
        uint64_t verusPreparedSeq = ~0ULL;
        uint32_t verusRemaining = 0;
        uint32_t verusNonce32 = 0;
        bool verusRollingNonce32 = false;
        bool verusDynamicNonce4 = false;
        bool verusSuffix7NoCross = false;
#       endif

        while (!Nonce::isOutdated(Nonce::CPU, m_job.sequence())) {
            const Job &job = m_job.currentJob();

            if (job.algorithm().l3() != m_algorithm.l3()) {
                break;
            }

#           ifdef XMRIG_ALGO_ASTROBWT
            if (N == 1 && job.algorithm().family() == Algorithm::ASTROBWT && !m_benchSize) {
                bool exhausted = false;
                const uint32_t reserveStep = kReserveCount;
                const uint64_t target = job.target();
                const uint64_t nonceMask = job.nonceMask();
                const uint32_t jobIndex = m_job.index();
                uint8_t *blob = m_job.blob();
                uint32_t *noncePtr = m_job.nonce();
                const size_t blobSize = job.size();
                const size_t inputLen = (blobSize < astrobwt::DERO_MINIBLOCK_SIZE)
                    ? blobSize
                    : static_cast<size_t>(astrobwt::DERO_MINIBLOCK_SIZE);
                uint32_t astroRemaining = reserveStep;
                uint32_t astroNonce32 = loadU32Hot(noncePtr);

                for (int lane = 0; lane < kAstroBatch; ++lane) {
                    astrobwt::single_hash<Algorithm::ASTROBWT_DERO_3>(blob, inputLen, m_hash, m_ctx, job.height());

                    if (XMRIG_UNLIKELY(loadU64Hot(m_hash + 24) < target)) {
                        JobResults::submit(job, astroNonce32, m_hash, nullptr);
                    }

                    ++m_count;

                    if (XMRIG_LIKELY(--astroRemaining != 0)) {
                        ++astroNonce32;
                        storeU32Hot(noncePtr, astroNonce32);
                    }
                    else {
                        if (!Nonce::next(jobIndex, noncePtr, reserveStep, nonceMask)) {
                            JobResults::done(job);
                            exhausted = true;
                            break;
                        }
                        astroRemaining = reserveStep;
                        astroNonce32 = loadU32Hot(noncePtr);
                    }
                }

                if (exhausted) {
                    break;
                }

                if (m_yield) {
                    std::this_thread::yield();
                }

                continue;
            }
#           endif

#           ifdef XMRIG_ALGO_VERUSHASH
            if (N == 1) {
                if (job.algorithm().family() == Algorithm::VERUSHASH_FAMILY && !m_benchSize) {
                    bool exhausted = false;
                    const int kVerusBatch = verusBatchSize();
                    const uint32_t kVerusReserve = verusReserveSize();
                    const bool kUseDirectNonce32 = verusUseDirectNonce32();
                    const bool kForceNonce32Prepared = verusForceNonce32Prepared();
                    const uint64_t seq = m_job.sequence();
                    const bool hasVerusTarget = job.hasVerusTarget();
                    const uint64_t legacyTarget = hasVerusTarget ? 0 : job.target();
                    uint8_t *blob = m_job.blob();
                    uint32_t *noncePtr = m_job.nonce();
                    const size_t blobSize = job.size();
                    if (verusPreparedSeq != seq) {
                        verushash::prepare_fast_job(blob, blobSize, seq);
                        verusPreparedSeq = seq;
                        verusRemaining = kVerusReserve;
                        verusNonce32 = loadU32Hot(noncePtr);
                        verusRollingNonce32 = kUseDirectNonce32 && verushash::prepared_hot_is_rolling_nonce32();
                        verusDynamicNonce4 = kUseDirectNonce32 && verushash::prepared_hot_is_dynamic_nonce4();
                        verusSuffix7NoCross = kUseDirectNonce32 && verushash::prepared_hot_is_suffix7_no_cross();
                    }
                    const uint32_t reserveStep = kVerusReserve;
                    const uint64_t nonceMask = job.nonceMask();
                    const uint32_t jobIndex = m_job.index();

                    if (!kUseDirectNonce32) {
                        if (hasVerusTarget) {
                            const uint64_t *target64 = job.verusTargetWords();
                            const uint64_t t0 = target64[0];
                            const uint64_t t1 = target64[1];
                            const uint64_t t2 = target64[2];
                            const uint64_t t3 = target64[3];

#define XMRIG_VERUS_CHECK_AND_STEP_PREPARED() \
                            const uint64_t h3 = loadU64Hot(m_hash + 24); \
                            bool hit = false; \
                            if (h3 < t3) { \
                                hit = true; \
                            } \
                            else if (h3 == t3) { \
                                const uint64_t h2 = loadU64Hot(m_hash + 16); \
                                if (h2 < t2) { \
                                    hit = true; \
                                } \
                                else if (h2 == t2) { \
                                    const uint64_t h1 = loadU64Hot(m_hash + 8); \
                                    if (h1 < t1) { \
                                        hit = true; \
                                    } \
                                    else if (h1 == t1) { \
                                        const uint64_t h0 = loadU64Hot(m_hash); \
                                        hit = (h0 <= t0); \
                                    } \
                                } \
                            } \
                            if (XMRIG_UNLIKELY(hit)) { \
                                JobResults::submit(job, verusNonce32, m_hash, nullptr); \
                            } \
                            ++m_count; \
                            if (XMRIG_LIKELY(--verusRemaining != 0)) { \
                                ++verusNonce32; \
                                if (!kForceNonce32Prepared) { \
                                    storeU32Hot(noncePtr, verusNonce32); \
                                } \
                            } \
                            else { \
                                if (kForceNonce32Prepared) { \
                                    storeU32Hot(noncePtr, verusNonce32); \
                                } \
                                if (!Nonce::next(jobIndex, noncePtr, reserveStep, nonceMask)) { \
                                    JobResults::done(job); \
                                    exhausted = true; \
                                    break; \
                                } \
                                verusRemaining = reserveStep; \
                                verusNonce32 = loadU32Hot(noncePtr); \
                            }

                            if (kForceNonce32Prepared) {
                                for (int lane = 0; lane < kVerusBatch; ++lane) {
                                    verushash::single_hash_fast_prepared_hot_nonce32(blob, verusNonce32, m_hash);
                                    XMRIG_VERUS_CHECK_AND_STEP_PREPARED()
                                }
                            }
                            else {
                                for (int lane = 0; lane < kVerusBatch; ++lane) {
                                    verushash::single_hash_fast_prepared_hot(blob, m_hash);
                                    XMRIG_VERUS_CHECK_AND_STEP_PREPARED()
                                }
                            }

#undef XMRIG_VERUS_CHECK_AND_STEP_PREPARED
                        }
                        else {
#define XMRIG_VERUS_CHECK_AND_STEP_PREPARED_LEGACY() \
                            const uint64_t value = loadU64Hot(m_hash + 24); \
                            if (XMRIG_UNLIKELY(value < legacyTarget)) { \
                                JobResults::submit(job, verusNonce32, m_hash, nullptr); \
                            } \
                            ++m_count; \
                            if (XMRIG_LIKELY(--verusRemaining != 0)) { \
                                ++verusNonce32; \
                                if (!kForceNonce32Prepared) { \
                                    storeU32Hot(noncePtr, verusNonce32); \
                                } \
                            } \
                            else { \
                                if (kForceNonce32Prepared) { \
                                    storeU32Hot(noncePtr, verusNonce32); \
                                } \
                                if (!Nonce::next(jobIndex, noncePtr, reserveStep, nonceMask)) { \
                                    JobResults::done(job); \
                                    exhausted = true; \
                                    break; \
                                } \
                                verusRemaining = reserveStep; \
                                verusNonce32 = loadU32Hot(noncePtr); \
                            }

                            if (kForceNonce32Prepared) {
                                for (int lane = 0; lane < kVerusBatch; ++lane) {
                                    verushash::single_hash_fast_prepared_hot_nonce32(blob, verusNonce32, m_hash);
                                    XMRIG_VERUS_CHECK_AND_STEP_PREPARED_LEGACY()
                                }
                            }
                            else {
                                for (int lane = 0; lane < kVerusBatch; ++lane) {
                                    verushash::single_hash_fast_prepared_hot(blob, m_hash);
                                    XMRIG_VERUS_CHECK_AND_STEP_PREPARED_LEGACY()
                                }
                            }

#undef XMRIG_VERUS_CHECK_AND_STEP_PREPARED_LEGACY
                        }

                        if (exhausted) {
                            break;
                        }

                        continue;
                    }

                    enum class VerusHashMode : uint8_t {
                        PreparedHot = 0,
                        DirectNonce32,
                        RollingNonce32,
                        DynamicNoCross,
                        DynamicCross
                    };

                    VerusHashMode verusMode = VerusHashMode::PreparedHot;
                    if (verusRollingNonce32) {
                        verusMode = VerusHashMode::RollingNonce32;
                    }
                    else if (verusDynamicNonce4) {
                        verusMode = verusSuffix7NoCross ? VerusHashMode::DynamicNoCross : VerusHashMode::DynamicCross;
                    }
                    else {
                        verusMode = VerusHashMode::DirectNonce32;
                    }
                    const bool kStorePreparedNonce = (verusMode == VerusHashMode::PreparedHot);

                    if (hasVerusTarget) {
                        const uint64_t *target64 = job.verusTargetWords();
                        const uint64_t t0 = target64[0];
                        const uint64_t t1 = target64[1];
                        const uint64_t t2 = target64[2];
                        const uint64_t t3 = target64[3];

#define XMRIG_VERUS_CHECK_AND_STEP() \
                            const uint64_t h3 = loadU64Hot(m_hash + 24); \
                            bool hit = false; \
                            if (h3 < t3) { \
                                hit = true; \
                            } \
                            else if (h3 == t3) { \
                                const uint64_t h2 = loadU64Hot(m_hash + 16); \
                                if (h2 < t2) { \
                                    hit = true; \
                                } \
                                else if (h2 == t2) { \
                                    const uint64_t h1 = loadU64Hot(m_hash + 8); \
                                    if (h1 < t1) { \
                                        hit = true; \
                                    } \
                                    else if (h1 == t1) { \
                                        const uint64_t h0 = loadU64Hot(m_hash); \
                                        hit = (h0 <= t0); \
                                    } \
                                } \
                            } \
                            if (XMRIG_UNLIKELY(hit)) { \
                                JobResults::submit(job, verusNonce32, m_hash, nullptr); \
                            } \
                            ++m_count; \
                            if (XMRIG_LIKELY(--verusRemaining != 0)) { \
                                ++verusNonce32; \
                                if (kStorePreparedNonce) { \
                                    storeU32Hot(noncePtr, verusNonce32); \
                                } \
                            } \
                            else { \
                                if (!Nonce::next(jobIndex, noncePtr, reserveStep, nonceMask)) { \
                                    JobResults::done(job); \
                                    exhausted = true; \
                                    break; \
                                } \
                                verusRemaining = reserveStep; \
                                verusNonce32 = loadU32Hot(noncePtr); \
                            }

                        if (verusMode == VerusHashMode::DirectNonce32) {
                            for (int lane = 0; lane < kVerusBatch; ++lane) {
                                verushash::single_hash_fast_prepared_hot_nonce32(blob, verusNonce32, m_hash);
                                XMRIG_VERUS_CHECK_AND_STEP()
                            }
                        }
                        else {
                            switch (verusMode) {
                            case VerusHashMode::PreparedHot:
                                for (int lane = 0; lane < kVerusBatch; ++lane) {
                                    verushash::single_hash_fast_prepared_hot(blob, m_hash);
                                    XMRIG_VERUS_CHECK_AND_STEP()
                                }
                                break;

                            case VerusHashMode::RollingNonce32:
                                for (int lane = 0; lane < kVerusBatch; ++lane) {
                                    verushash::single_hash_fast_prepared_hot_nonce32_rolling(blob, verusNonce32, m_hash);
                                    XMRIG_VERUS_CHECK_AND_STEP()
                                }
                                break;

                            case VerusHashMode::DynamicNoCross:
                                for (int lane = 0; lane < kVerusBatch; ++lane) {
                                    verushash::single_hash_fast_prepared_hot_nonce32_dynamic_no_cross(verusNonce32, m_hash);
                                    XMRIG_VERUS_CHECK_AND_STEP()
                                }
                                break;

                            case VerusHashMode::DynamicCross:
                                for (int lane = 0; lane < kVerusBatch; ++lane) {
                                    verushash::single_hash_fast_prepared_hot_nonce32_dynamic_cross(verusNonce32, m_hash);
                                    XMRIG_VERUS_CHECK_AND_STEP()
                                }
                                break;

                            case VerusHashMode::DirectNonce32:
                            default:
                                break;
                            }
                        }

#undef XMRIG_VERUS_CHECK_AND_STEP
                    }
                    else {
                        
#define XMRIG_VERUS_CHECK_AND_STEP_LEGACY() \
                            const uint64_t value = loadU64Hot(m_hash + 24); \
                            if (XMRIG_UNLIKELY(value < legacyTarget)) { \
                                JobResults::submit(job, verusNonce32, m_hash, nullptr); \
                            } \
                            ++m_count; \
                            if (XMRIG_LIKELY(--verusRemaining != 0)) { \
                                ++verusNonce32; \
                                if (kStorePreparedNonce) { \
                                    storeU32Hot(noncePtr, verusNonce32); \
                                } \
                            } \
                            else { \
                                if (!Nonce::next(jobIndex, noncePtr, reserveStep, nonceMask)) { \
                                    JobResults::done(job); \
                                    exhausted = true; \
                                    break; \
                                } \
                                verusRemaining = reserveStep; \
                                verusNonce32 = loadU32Hot(noncePtr); \
                            }

                        if (verusMode == VerusHashMode::DirectNonce32) {
                            for (int lane = 0; lane < kVerusBatch; ++lane) {
                                verushash::single_hash_fast_prepared_hot_nonce32(blob, verusNonce32, m_hash);
                                XMRIG_VERUS_CHECK_AND_STEP_LEGACY()
                            }
                        }
                        else {
                            switch (verusMode) {
                            case VerusHashMode::PreparedHot:
                                for (int lane = 0; lane < kVerusBatch; ++lane) {
                                    verushash::single_hash_fast_prepared_hot(blob, m_hash);
                                    XMRIG_VERUS_CHECK_AND_STEP_LEGACY()
                                }
                                break;

                            case VerusHashMode::RollingNonce32:
                                for (int lane = 0; lane < kVerusBatch; ++lane) {
                                    verushash::single_hash_fast_prepared_hot_nonce32_rolling(blob, verusNonce32, m_hash);
                                    XMRIG_VERUS_CHECK_AND_STEP_LEGACY()
                                }
                                break;

                            case VerusHashMode::DynamicNoCross:
                                for (int lane = 0; lane < kVerusBatch; ++lane) {
                                    verushash::single_hash_fast_prepared_hot_nonce32_dynamic_no_cross(verusNonce32, m_hash);
                                    XMRIG_VERUS_CHECK_AND_STEP_LEGACY()
                                }
                                break;

                            case VerusHashMode::DynamicCross:
                                for (int lane = 0; lane < kVerusBatch; ++lane) {
                                    verushash::single_hash_fast_prepared_hot_nonce32_dynamic_cross(verusNonce32, m_hash);
                                    XMRIG_VERUS_CHECK_AND_STEP_LEGACY()
                                }
                                break;

                            case VerusHashMode::DirectNonce32:
                            default:
                                break;
                            }
                        }

#undef XMRIG_VERUS_CHECK_AND_STEP_LEGACY
                    }

                    if (exhausted) {
                        break;
                    }

                    // Verus hot path performs better on this target when avoiding per-batch yields.

                    continue;
                }
            }
#           endif

            uint64_t current_job_nonces[N];
            for (size_t i = 0; i < N; ++i) {
                if (job.nonceSize() == sizeof(uint64_t)) {
                    current_job_nonces[i] = readUnaligned(reinterpret_cast<const uint64_t *>(m_job.nonce(i)));
                }
                else {
                current_job_nonces[i] = readUnaligned(m_job.nonce(i));
                }
            }

#           ifdef XMRIG_FEATURE_BENCHMARK
            if (m_benchSize) {
                if (current_job_nonces[0] >= m_benchSize) {
                    return BenchState::done();
                }

                // Make each hash dependent on the previous one in single thread benchmark to prevent cheating with multiple threads
                if (m_threads == 1) {
                    *(uint64_t*)(m_job.blob()) ^= BenchState::data();
                }
            }
#           endif

            bool valid = true;

            uint8_t miner_signature_saved[64];

#           ifdef XMRIG_ALGO_RANDOMX
            uint8_t* miner_signature_ptr = m_job.blob() + m_job.nonceOffset() + m_job.nonceSize();
            if (job.algorithm().family() == Algorithm::RANDOM_X) {
                if (first) {
                    first = false;
                    if (job.hasMinerSignature()) {
                        job.generateMinerSignature(m_job.blob(), job.size(), miner_signature_ptr);
                    }
                    randomx_calculate_hash_first(m_vm, tempHash, m_job.blob(), job.size());
                }

                if (!nextRound()) {
                    break;
                }

                if (job.hasMinerSignature()) {
                    memcpy(miner_signature_saved, miner_signature_ptr, sizeof(miner_signature_saved));
                    job.generateMinerSignature(m_job.blob(), job.size(), miner_signature_ptr);
                }
                randomx_calculate_hash_next(m_vm, tempHash, m_job.blob(), job.size(), m_hash);
            }
            else
#           endif
            {
                switch (job.algorithm().family()) {

#               ifdef XMRIG_ALGO_GHOSTRIDER
                case Algorithm::GHOSTRIDER:
                    if (N == 8) {
                        ghostrider::hash_octa(m_job.blob(), job.size(), m_hash, m_ctx, m_ghHelper);
                    }
                    else {
                        valid = false;
                    }
                    break;
#               endif
#               ifdef XMRIG_ALGO_VERUSHASH
                case Algorithm::VERUSHASH_FAMILY:
                    verushash::single_hash_fast(m_job.blob(), job.size(), m_hash, m_job.sequence());
                    break;
#               endif

#               ifdef XMRIG_ALGO_NM
                case Algorithm::NM:
                    nm_fast_hash(m_nmEpoch, &m_nmLane, m_job.blob(), job.height(), m_hash);
                    break;
#               endif

                default:
                    fn(job.algorithm())(m_job.blob(), job.size(), m_hash, m_ctx, job.height());
                    break;
                }

                if (!nextRound()) {
                    break;
                };
            }

            if (valid) {
                for (size_t i = 0; i < N; ++i) {
#                   ifdef XMRIG_ALGO_NM
                    if (job.algorithm().family() == Algorithm::NM) {
                        if (memcmp(m_hash + (i * 32), job.target32(), 32) <= 0) {
                            const uint64_t nmNonce = static_cast<uint64_t>(current_job_nonces[i]) |
                                (static_cast<uint64_t>(readUnaligned(m_job.nonce(i) + 1)) << 32);
                            JobResults::submit(JobResult(job, nmNonce, m_hash + (i * 32)));
                        }
                        continue;
                    }
#                   endif

                    const uint64_t value = readUnaligned(reinterpret_cast<const uint64_t *>(m_hash + (i * 32) + 24));
                    bool hit = false;

#                   ifdef XMRIG_FEATURE_BENCHMARK
                    if (m_benchSize) {
                        if (current_job_nonces[i] < m_benchSize) {
                            BenchState::add(value);
                        }
                    }
                    else
#                   endif
#                   ifdef XMRIG_ALGO_VERUSHASH
                    if (job.algorithm().family() == Algorithm::VERUSHASH_FAMILY && job.hasVerusTarget()) {
                        hit = job.isVerusTargetReached(m_hash + (i * 32));
                    }
                    else
#                   endif
                    {
                        hit = value < job.target();
                    }

                    if (hit) {
                        JobResults::submit(job, current_job_nonces[i], m_hash + (i * 32), job.hasMinerSignature() ? miner_signature_saved : nullptr);
                    }
                }
                m_count += N;
            }

#           ifdef XMRIG_ALGO_NM
            if (job.algorithm().family() == Algorithm::NM) {
                continue;
            }
#           endif

            if (m_yield) {
                std::this_thread::yield();
            }
        }

        if (!Nonce::isPaused()) {
            consumeJob();
        }
    }
}


template<size_t N>
bool xmrig::CpuWorker<N>::nextRound()
{
#   ifdef XMRIG_FEATURE_BENCHMARK
    const uint32_t count = m_benchSize ? 1U : kReserveCount;
#   else
    constexpr uint32_t count = kReserveCount;
#   endif

    if (!m_job.nextRound(count, 1)) {
        JobResults::done(m_job.currentJob());

        return false;
    }

    return true;
}


template<size_t N>
bool xmrig::CpuWorker<N>::verify(const Algorithm &algorithm, const uint8_t *referenceValue)
{
#   ifdef XMRIG_ALGO_GHOSTRIDER
    if (algorithm == Algorithm::GHOSTRIDER_RTM) {
        uint8_t blob[N * 80] = {};
        for (size_t i = 0; i < N; ++i) {
            blob[i * 80 + 0] = static_cast<uint8_t>(i);
            blob[i * 80 + 4] = 0x10;
            blob[i * 80 + 5] = 0x02;
        }

        uint8_t hash1[N * 32] = {};
        ghostrider::hash_octa(blob, 80, hash1, m_ctx, 0, false);

        for (size_t i = 0; i < N; ++i) {
            blob[i * 80 + 0] = static_cast<uint8_t>(i);
            blob[i * 80 + 4] = 0x43;
            blob[i * 80 + 5] = 0x05;
        }

        uint8_t hash2[N * 32] = {};
        ghostrider::hash_octa(blob, 80, hash2, m_ctx, 0, false);

        for (size_t i = 0; i < N * 32; ++i) {
            if ((hash1[i] ^ hash2[i]) != referenceValue[i]) {
                return false;
            }
        }

        return true;
    }
#   endif

    cn_hash_fun func = fn(algorithm);
    if (!func) {
        return false;
    }

    func(test_input, 76, m_hash, m_ctx, 0);
    return memcmp(m_hash, referenceValue, sizeof m_hash) == 0;
}


template<size_t N>
bool xmrig::CpuWorker<N>::verify2(const Algorithm &algorithm, const uint8_t *referenceValue)
{
    cn_hash_fun func = fn(algorithm);
    if (!func) {
        return false;
    }

    for (size_t i = 0; i < (sizeof(cn_r_test_input) / sizeof(cn_r_test_input[0])); ++i) {
        const size_t size = cn_r_test_input[i].size;
        for (size_t k = 0; k < N; ++k) {
            memcpy(m_job.blob() + (k * size), cn_r_test_input[i].data, size);
        }

        func(m_job.blob(), size, m_hash, m_ctx, cn_r_test_input[i].height);

        for (size_t k = 0; k < N; ++k) {
            if (memcmp(m_hash + k * 32, referenceValue + i * 32, sizeof m_hash / N) != 0) {
                return false;
            }
        }
    }

    return true;
}


namespace xmrig {

template<>
bool CpuWorker<1>::verify2(const Algorithm &algorithm, const uint8_t *referenceValue)
{
    cn_hash_fun func = fn(algorithm);
    if (!func) {
        return false;
    }

    for (size_t i = 0; i < (sizeof(cn_r_test_input) / sizeof(cn_r_test_input[0])); ++i) {
        func(cn_r_test_input[i].data, cn_r_test_input[i].size, m_hash, m_ctx, cn_r_test_input[i].height);

        if (memcmp(m_hash, referenceValue + i * 32, sizeof m_hash) != 0) {
            return false;
        }
    }

    return true;
}

} // namespace xmrig


template<size_t N>
void xmrig::CpuWorker<N>::allocateCnCtx()
{
    if (m_ctx[0] == nullptr) {
        int shift = 0;

#       ifdef XMRIG_ALGO_CN_HEAVY
        // cn-heavy optimization for Zen3 CPUs
        if (m_memory == cn_heavyZen3Memory) {
            shift = (id() / 8) * m_algorithm.l3() * 8 + (id() % 8) * 64;
        }
#       endif

        CnCtx::create(m_ctx, m_memory->scratchpad() + shift, m_algorithm.l3(), N);
    }
}


template<size_t N>
void xmrig::CpuWorker<N>::consumeJob()
{
    if (Nonce::sequence(Nonce::CPU) == 0) {
        return;
    }

    auto job = m_miner->job();

#   ifdef XMRIG_FEATURE_BENCHMARK
    m_benchSize          = job.benchSize();
    uint32_t count = m_benchSize ? 1U : kReserveCount;
#   else
    uint32_t count = kReserveCount;
#   endif

#   ifdef XMRIG_ALGO_VERUSHASH
    bool isBench = false;
#       ifdef XMRIG_FEATURE_BENCHMARK
    isBench = (m_benchSize != 0);
#       endif
    if (!isBench && job.algorithm().family() == Algorithm::VERUSHASH_FAMILY) {
        // Keep initial reservation size aligned with fast-path stepping size.
        count = verusReserveSize();
    }
#   endif

    m_job.add(job, count, Nonce::CPU);

#   ifdef XMRIG_ALGO_RANDOMX
    if (m_job.currentJob().algorithm().family() == Algorithm::RANDOM_X) {
        allocateRandomX_VM();
    }
    else
#   endif
#   ifdef XMRIG_ALGO_NM
    if (m_job.currentJob().algorithm().family() == Algorithm::NM) {
        allocateNm();
    }
    else
#   endif
    {
        allocateCnCtx();
    }
}


namespace xmrig {

template class CpuWorker<1>;
template class CpuWorker<2>;
template class CpuWorker<3>;
template class CpuWorker<4>;
template class CpuWorker<5>;
template class CpuWorker<8>;

} // namespace xmrig

