/* DRQ Miner — OpenCL VerusHash GPU path (monkins1010/AMDVerusCoin input.cl). */

#include "backend/opencl/runners/OclVerusHashRunner.h"

#include "backend/common/Tags.h"
#include "backend/opencl/OclCache.h"
#include "backend/opencl/OclLaunchData.h"
#include "backend/opencl/wrappers/OclDevice.h"
#include "backend/opencl/wrappers/OclError.h"
#include "backend/opencl/wrappers/OclLib.h"
#include "base/io/log/Log.h"
#include "base/io/log/Tags.h"
#include "base/tools/Alignment.h"
#include "crypto/common/Nonce.h"
#include "crypto/verushash/VerusHash_xmrig.h"
#include "crypto/verushash/cuda/VerusCudaBridge.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <stdexcept>


namespace xmrig {


static constexpr size_t kVerusPreimageSize  = 1487;
static constexpr size_t kVerusKeyBytes      = 8832;
static constexpr size_t kVerusWorkSize      = 128;
static constexpr uint32_t kVerusSlotsMax    = 0xffff;


uint32_t OclVerusHashRunner::verusSlotCount(uint32_t intensity)
{
    uint32_t slots = intensity > 0 ? intensity : 1;
    slots--;
    slots |= slots >> 1;
    slots |= slots >> 2;
    slots |= slots >> 4;
    slots |= slots >> 8;
    slots |= slots >> 16;
    slots++;

    if (slots > kVerusSlotsMax) {
        slots = kVerusSlotsMax;
    }

    return slots;
}


uint32_t OclVerusHashRunner::verusIntensityForDevice(const OclDevice &device)
{
    constexpr size_t kKeyBytes      = kVerusKeyBytes;
    constexpr uint32_t kMinIntensity = 128;

    const size_t mem = device.freeMemSize();
    uint32_t maxSlots = kVerusSlotsMax;

    if (mem > kKeyBytes) {
        const size_t budget = (mem * 4) / 5;
        maxSlots = static_cast<uint32_t>(std::min<size_t>(kVerusSlotsMax, budget / kKeyBytes));
        maxSlots |= maxSlots >> 1;
        maxSlots |= maxSlots >> 2;
        maxSlots |= maxSlots >> 4;
        maxSlots |= maxSlots >> 8;
        maxSlots |= maxSlots >> 16;
        maxSlots = (maxSlots + 1) >> 1;

        if (maxSlots < kMinIntensity) {
            maxSlots = kMinIntensity;
        }
    }

    uint32_t intensity;

#   if defined(XMRIG_OS_ANDROID)
    intensity = std::max(device.computeUnits() * 256U, kMinIntensity);
#   else
    if (mem > 0 && mem < 2ULL * 1024 * 1024 * 1024) {
        intensity = std::max(device.computeUnits() * 512U, 1024U);
    }
    else {
        intensity = std::max(device.computeUnits() * 2048U, 4096U);
    }
#   endif

    while (verusSlotCount(intensity) > maxSlots && intensity > kMinIntensity) {
        intensity /= 2;
    }

    return std::max(intensity, kMinIntensity);
}


OclVerusHashRunner::OclVerusHashRunner(size_t index, const OclLaunchData &data) :
    OclBaseRunner(index, data)
{
    m_options += " -DTHREADS=128";

    if (data.device.vendorId() == OclVendor::OCL_VENDOR_AMD) {
        m_options += " -DPLATFORM=OPENCL_PLATFORM_AMD";
    }
    else if (data.device.vendorId() == OclVendor::OCL_VENDOR_NVIDIA) {
        m_options += " -DPLATFORM=OPENCL_PLATFORM_NVIDIA";
    }

    (void) index;
}


OclVerusHashRunner::~OclVerusHashRunner()
{
    OclLib::release(m_kernel);
    OclLib::release(m_longKeys);
    OclLib::release(m_resNonce);
    OclLib::release(m_target);
    OclLib::release(m_vkey);
    OclLib::release(m_blockhash);
    OclLib::release(m_startNonce);
}


void OclVerusHashRunner::init()
{
    m_queue = OclLib::createCommandQueue(m_ctx, data().device.id());

    m_slots = verusSlotCount(static_cast<uint32_t>(m_intensity));

    char totalMaxBuf[32];
    std::snprintf(totalMaxBuf, sizeof(totalMaxBuf), " -DTOTAL_MAX=0x%x", m_slots - 1);
    m_options += totalMaxBuf;

    const size_t longKeysSize = static_cast<size_t>(m_slots) * kVerusKeyBytes;

    cl_int err = CL_SUCCESS;

    auto makeBuffer = [&](cl_mem_flags flags, size_t size) {
        cl_int e = CL_SUCCESS;
        cl_mem buf = OclLib::createBuffer(m_ctx, flags, size, nullptr, &e);
        if (e != CL_SUCCESS) {
            throw std::runtime_error(OclError::toString(e));
        }
        return buf;
    };

    m_startNonce = makeBuffer(CL_MEM_READ_ONLY, sizeof(cl_uint));
    m_blockhash  = makeBuffer(CL_MEM_READ_ONLY, 64);
    m_vkey       = makeBuffer(CL_MEM_READ_ONLY, kVerusKeyBytes);
    m_target     = makeBuffer(CL_MEM_READ_ONLY, 8 * sizeof(cl_uint));
    m_resNonce   = makeBuffer(CL_MEM_READ_WRITE, sizeof(cl_uint));
    m_longKeys   = makeBuffer(CL_MEM_READ_WRITE, longKeysSize);

    m_gpuReady = true;
}


void OclVerusHashRunner::build()
{
    m_program = OclCache::build(this);

    if (!m_program) {
        throw std::runtime_error(OclError::toString(CL_INVALID_PROGRAM));
    }

    m_kernel = OclLib::createKernel(m_program, "verus_gpu_hash");
}


uint32_t OclVerusHashRunner::processedHashes() const
{
    return static_cast<uint32_t>(m_intensity);
}


void OclVerusHashRunner::set(const Job &job, uint8_t *blob)
{
    m_job  = job;
    m_blob = blob;

    verushash::prepare_fast_job(blob, job.size(), Nonce::sequence(Nonce::OPENCL));

    m_useGpu   = false;
    m_gpuJobOk = false;

    if (!m_gpuReady || !m_kernel || !job.hasVerusTarget() || job.size() != kVerusPreimageSize || !m_blob) {
        return;
    }

    const uint8_t *sol = m_blob + 140 + 3;
    const uint8_t ver  = sol[0];
    const bool mergedMm = (ver >= 7 && sol[5] > 0);
    if (mergedMm) {
        return;
    }

    alignas(64) uint8_t full[kVerusPreimageSize];
    std::memcpy(full, m_blob, kVerusPreimageSize);

    uint8_t nonceSpace[15] = {};

    alignas(64) uint8_t blockhash_half[64];
    verus_cuda_VerusHashHalf(blockhash_half, full, static_cast<int>(kVerusPreimageSize));

    alignas(64) uint8_t data_key[kVerusKeyBytes];
    verus_cuda_GenNewCLKey(blockhash_half, data_key);
    std::memcpy(blockhash_half + 32, nonceSpace, 15);

    std::memcpy(m_ptarget, job.verusTargetBytes(), sizeof(m_ptarget));
    m_version = full[143];

    enqueueWriteBuffer(m_blockhash, CL_TRUE, 0, 64, blockhash_half);
    enqueueWriteBuffer(m_vkey, CL_TRUE, 0, kVerusKeyBytes, data_key);
    enqueueWriteBuffer(m_target, CL_TRUE, 0, 8 * sizeof(cl_uint), m_ptarget);

    m_useGpu   = true;
    m_gpuJobOk = true;
}


void OclVerusHashRunner::run(uint32_t nonce, uint32_t /*nonce_offset*/, uint32_t *hashOutput)
{
    hashOutput[0xFF] = 0;

    if (!m_useGpu || !m_gpuJobOk) {
        alignas(16) uint8_t hash[32]{};
        uint32_t *const nonce_ptr = reinterpret_cast<uint32_t *>(m_blob + m_job.nonceOffset());
        uint32_t found            = 0;

        for (uint32_t h = 0; h < m_intensity && found < 15; ++h) {
            writeUnaligned(nonce_ptr, nonce + h);
            verushash::single_hash_fast_prepared_unchecked(m_blob, m_job.size(), hash);

            bool hit = false;
            if (m_job.hasVerusTarget()) {
                hit = m_job.isVerusTargetReached(hash);
            }
            else {
                hit = (readUnaligned(reinterpret_cast<const uint64_t *>(hash + 24)) < m_job.target());
            }

            if (hit) {
                hashOutput[++found] = nonce + h;
            }
        }

        hashOutput[0xFF] = found;
        return;
    }

    const cl_uint start = nonce;
    const cl_uint invalid = 0xffffffffU;

    enqueueWriteBuffer(m_startNonce, CL_FALSE, 0, sizeof(cl_uint), &start);
    enqueueWriteBuffer(m_resNonce, CL_FALSE, 0, sizeof(cl_uint), &invalid);

    const size_t gsize = ((m_intensity + kVerusWorkSize - 1) / kVerusWorkSize) * kVerusWorkSize;
    const size_t lsize = kVerusWorkSize;

    OclLib::setKernelArg(m_kernel, 0, sizeof(cl_mem), &m_startNonce);
    OclLib::setKernelArg(m_kernel, 1, sizeof(cl_mem), &m_blockhash);
    OclLib::setKernelArg(m_kernel, 2, sizeof(cl_mem), &m_longKeys);
    OclLib::setKernelArg(m_kernel, 3, sizeof(cl_mem), &m_vkey);
    OclLib::setKernelArg(m_kernel, 4, sizeof(cl_mem), &m_target);
    OclLib::setKernelArg(m_kernel, 5, sizeof(cl_mem), &m_resNonce);

    const size_t global_offset = nonce;
    const cl_int ret = OclLib::enqueueNDRangeKernel(m_queue, m_kernel, 1, &global_offset, &gsize, &lsize, 0, nullptr, nullptr);
    if (ret != CL_SUCCESS) {
        LOG_ERR("%s" RED(" error ") RED_BOLD("%s") RED(" when calling ") RED_BOLD("clEnqueueNDRangeKernel") RED(" for ")
                    RED_BOLD("verus_gpu_hash"),
                Tags::opencl(), OclError::toString(ret));
        throw std::runtime_error(OclError::toString(ret));
    }

    cl_uint result = invalid;
    enqueueReadBuffer(m_resNonce, CL_TRUE, 0, sizeof(cl_uint), &result);

    if (result != invalid) {
        hashOutput[1]   = result;
        hashOutput[0xFF] = 1;
    }
}


} // namespace xmrig
