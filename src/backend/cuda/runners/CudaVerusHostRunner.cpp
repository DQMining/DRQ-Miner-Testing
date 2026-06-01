#include "backend/cuda/runners/CudaVerusHostRunner.h"
#include "backend/common/Tags.h"
#include "backend/cuda/wrappers/CudaDevice.h"
#include "base/io/log/Log.h"
#include "base/tools/Alignment.h"
#include "crypto/common/Nonce.h"
#include "crypto/verushash/VerusHash_xmrig.h"

#   ifdef XMRIG_FEATURE_CUDA_VERUS
#       include "crypto/verushash/cuda/VerusCudaBridge.h"
#       include "crypto/verushash/cuda/verus_cuda_max_gpus.h"
#       include <array>
#       include <cuda_runtime.h>
#       include <mutex>
#   endif

#include <cstring>


static constexpr size_t kVerusPreimageSize = 1487;

#   ifdef XMRIG_FEATURE_CUDA_VERUS
static std::array<std::mutex, MAX_GPUS> s_verusDeviceMutex;

static std::mutex &verusDeviceMutex(int deviceIndex)
{
    if (deviceIndex < 0 || deviceIndex >= MAX_GPUS) {
        return s_verusDeviceMutex[0];
    }
    return s_verusDeviceMutex[static_cast<size_t>(deviceIndex)];
}
#   endif

static uint32_t verusCudaIntensity(const xmrig::CudaLaunchData &launch)
{
    const size_t b  = static_cast<size_t>(launch.thread.blocks());
    const size_t t  = static_cast<size_t>(launch.thread.threads());
    const size_t nb = b > 0 ? b : size_t{1};
    const size_t nt = t > 0 ? t : size_t{1};
    return static_cast<uint32_t>(nb * nt);
}

xmrig::CudaVerusHostRunner::CudaVerusHostRunner(size_t id, const xmrig::CudaLaunchData &launch) :
    m_data(launch),
    m_intensity(verusCudaIntensity(launch)),
    m_thrId(static_cast<int>(launch.device.index()))
{
    (void) id;
}


xmrig::CudaVerusHostRunner::~CudaVerusHostRunner()
{
#   ifdef XMRIG_FEATURE_CUDA_VERUS
    if (m_cudaInit) {
        std::lock_guard<std::mutex> lock(verusDeviceMutex(m_thrId));
        cudaSetDevice(static_cast<int>(m_data.device.index()));
        xmrig_verus_cuda_cleanup(m_thrId);
        m_cudaInit = false;
    }
#   endif
}


size_t xmrig::CudaVerusHostRunner::intensity() const
{
    return m_intensity;
}


bool xmrig::CudaVerusHostRunner::init()
{
    verushash::init();

#   ifdef XMRIG_FEATURE_CUDA_VERUS
    if (m_thrId < 0 || m_thrId >= MAX_GPUS) {
        return false;
    }
    if (cudaSetDevice(static_cast<int>(m_data.device.index())) != cudaSuccess) {
        return false;
    }
    if (xmrig_verus_cuda_init(m_thrId, m_intensity) != 0) {
        LOG_WARN("%s " RED_BOLD("Verus GPU init failed") YELLOW(" (CUDA allocation or device error). Check stderr for cudaMalloc details; this thread is disabled."),
                 cuda_tag());
        return false;
    }
    m_cudaInit = true;
#   endif

    return true;
}


bool xmrig::CudaVerusHostRunner::set(const Job &job, uint8_t *blob)
{
    m_job  = job;
    m_blob = blob;
    verushash::prepare_fast_job(blob, job.size(), Nonce::sequence(Nonce::CUDA));

#   ifdef XMRIG_FEATURE_CUDA_VERUS
    m_gpuJobOk = false;
    m_useGpu   = false;

    if (m_cudaInit && job.hasVerusTarget() && job.size() == kVerusPreimageSize && m_blob) {
        const uint8_t *sol = m_blob + 140 + 3;
        const uint8_t ver  = sol[0];
        const bool mergedMm = (ver >= 7 && sol[5] > 0);
        if (!mergedMm) {
            m_useGpu = true;
        }
    }

    if (m_useGpu) {
        alignas(64) uint8_t full[kVerusPreimageSize];
        memcpy(full, m_blob, kVerusPreimageSize);

        uint8_t nonceSpace[15] = {};

        alignas(64) uint8_t blockhash_half[64];
        verus_cuda_VerusHashHalf(blockhash_half, full, static_cast<int>(kVerusPreimageSize));

        alignas(64) uint8_t data_key[8832];
        verus_cuda_GenNewCLKey(blockhash_half, data_key);
        memcpy(blockhash_half + 32, nonceSpace, 15);

        memcpy(m_ptarget, job.verusTargetBytes(), sizeof(m_ptarget));
        m_version = full[143];

        if (cudaSetDevice(static_cast<int>(m_data.device.index())) != cudaSuccess) {
            m_useGpu   = false;
            m_gpuJobOk = false;
            return true;
        }

        {
            std::lock_guard<std::mutex> lock(verusDeviceMutex(m_thrId));
            xmrig_verus_cuda_set_block(blockhash_half, m_ptarget, data_key, m_thrId, m_intensity);
        }
        m_gpuJobOk = true;
    }
#   endif

    return true;
}


bool xmrig::CudaVerusHostRunner::run(uint32_t startNonce, uint32_t *rescount, uint32_t *resnonce)
{
#   ifdef XMRIG_FEATURE_CUDA_VERUS
    if (m_useGpu && m_gpuJobOk) {
        if (cudaSetDevice(static_cast<int>(m_data.device.index())) != cudaSuccess) {
            *rescount = 0;
            return false;
        }
        uint32_t r = UINT32_MAX;
        {
            std::lock_guard<std::mutex> lock(verusDeviceMutex(m_thrId));
            xmrig_verus_cuda_hash(m_thrId, m_intensity, startNonce, &r, m_version);
            cudaDeviceSynchronize();
        }
        if (r != UINT32_MAX) {
            resnonce[0] = r;
            *rescount   = 1;
        }
        else {
            *rescount = 0;
        }
        return true;
    }
#   endif

    alignas(16) uint8_t hash[32]{};
    uint32_t *const nonce_ptr = reinterpret_cast<uint32_t *>(m_blob + m_job.nonceOffset());
    uint32_t found              = 0;

    for (uint32_t h = 0; h < m_intensity && found < 16; ++h) {
        writeUnaligned(nonce_ptr, startNonce + h);
        verushash::single_hash_fast_prepared_unchecked(m_blob, m_job.size(), hash);

        bool hit = false;
        if (m_job.hasVerusTarget()) {
            hit = m_job.isVerusTargetReached(hash);
        }
        else {
            hit = (readUnaligned(reinterpret_cast<const uint64_t *>(hash + 24)) < m_job.target());
        }

        if (hit) {
            resnonce[found++] = startNonce + h;
        }
    }

    *rescount = found;

    return true;
}
