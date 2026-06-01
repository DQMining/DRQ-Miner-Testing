/* XMRig
 * Copyright 2010      Jeff Garzik <jgarzik@pobox.com>
 * Copyright 2012-2014 pooler      <pooler@litecoinpool.org>
 * Copyright 2014      Lucas Jones <https://github.com/lucasjones>
 * Copyright 2014-2016 Wolf9466    <https://github.com/OhGodAPet>
 * Copyright 2016      Jay D Dee   <jayddee246@gmail.com>
 * Copyright 2017-2018 XMR-Stak    <https://github.com/fireice-uk>, <https://github.com/psychocrypt>
 * Copyright 2018-2024 SChernykh   <https://github.com/SChernykh>
 * Copyright 2016-2024 XMRig       <https://github.com/xmrig>, <support@xmrig.com>
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

#include "backend/cuda/wrappers/CudaDevice.h"
#include "3rdparty/rapidjson/document.h"
#include "backend/cuda/CudaThreads.h"
#include "backend/cuda/wrappers/CudaLib.h"
#include "base/crypto/Algorithm.h"
#include "base/io/log/Log.h"


#ifdef XMRIG_FEATURE_NVML
#   include "backend/cuda/wrappers/NvmlLib.h"
#endif

#include <algorithm>
#include <vector>


#   ifdef XMRIG_FEATURE_CUDA_VERUS
#       include <cuda_runtime.h>
#   endif


#   ifdef XMRIG_FEATURE_CUDA_VERUS
xmrig::CudaDevice::CudaDevice(NativeCudaProbeTag, uint32_t index, int32_t /*bfactor*/, int32_t /*bsleep*/) :
    m_index(index)
{
    cudaDeviceProp prop{};
    if (cudaSetDevice(static_cast<int>(index)) != cudaSuccess) {
        return;
    }
    if (cudaGetDeviceProperties(&prop, static_cast<int>(index)) != cudaSuccess) {
        return;
    }

    m_nativeRuntime = true;
    m_name          = prop.name;
    m_nativeSmx     = static_cast<uint32_t>(prop.multiProcessorCount);
    // CUDA 13+ removed clockRate / memoryClockRate from cudaDeviceProp; attributes still work.
    int clockKhz = 0;
    int memKhz   = 0;
    const int di = static_cast<int>(index);
    cudaDeviceGetAttribute(&clockKhz, cudaDevAttrClockRate, di);
    cudaDeviceGetAttribute(&memKhz, cudaDevAttrMemoryClockRate, di);
    m_nativeClockKhz    = static_cast<uint32_t>(clockKhz);
    m_nativeMemClockKhz = static_cast<uint32_t>(memKhz);
    m_nativeArchMajor   = static_cast<uint32_t>(prop.major);
    m_nativeArchMinor   = static_cast<uint32_t>(prop.minor);
    m_nativeTotalMem    = static_cast<size_t>(prop.totalGlobalMem);

    /* Without this, topology stays invalid but bus()/device() read 0 — NvmlLib::assign matches every GPU to PCI 0:00.0. */
    int pci_bus = 0;
    int pci_dev = 0;
    if (cudaDeviceGetAttribute(&pci_bus, cudaDevAttrPciBusId, di) == cudaSuccess &&
        cudaDeviceGetAttribute(&pci_dev, cudaDevAttrPciDeviceId, di) == cudaSuccess) {
        m_topology = PciTopology(pci_bus, pci_dev, 0);
    }
}


std::vector<xmrig::CudaDevice> xmrig::CudaDevice::enumerateEmbeddedCudaDevices(int32_t bfactor, int32_t bsleep)
{
    std::vector<CudaDevice> out;
    int                      n = 0;
    if (cudaGetDeviceCount(&n) != cudaSuccess || n <= 0) {
        return out;
    }

    for (int i = 0; i < n; ++i) {
        CudaDevice dev(NativeCudaProbeTag{}, static_cast<uint32_t>(i), bfactor, bsleep);
        if (dev.isValid()) {
            out.push_back(std::move(dev));
        }
    }

    return out;
}
#   endif


xmrig::CudaDevice::CudaDevice(uint32_t index, int32_t bfactor, int32_t bsleep) :
    m_index(index)
{
    auto *ctx = CudaLib::alloc(index, bfactor, bsleep);
    if (!CudaLib::deviceInfo(ctx, 0, 0, Algorithm::INVALID)) {
        CudaLib::release(ctx);

        return;
    }

    m_ctx       = ctx;
    m_name      = CudaLib::deviceName(ctx);
    m_topology  = { CudaLib::deviceUint(ctx, CudaLib::DevicePciBusID), CudaLib::deviceUint(ctx, CudaLib::DevicePciDeviceID), 0U };
}


xmrig::CudaDevice::CudaDevice(CudaDevice &&other) noexcept :
    m_index(other.m_index),
    m_ctx(other.m_ctx),
    m_topology(other.m_topology),
    m_name(std::move(other.m_name)),
    m_nativeRuntime(other.m_nativeRuntime),
    m_nativeSmx(other.m_nativeSmx),
    m_nativeClockKhz(other.m_nativeClockKhz),
    m_nativeMemClockKhz(other.m_nativeMemClockKhz),
    m_nativeArchMajor(other.m_nativeArchMajor),
    m_nativeArchMinor(other.m_nativeArchMinor),
    m_nativeTotalMem(other.m_nativeTotalMem)
{
    other.m_ctx           = nullptr;
    other.m_nativeRuntime = false;
}


xmrig::CudaDevice::~CudaDevice()
{
    /* Embedded Verus uses CUDA runtime only: m_ctx is null, m_nativeRuntime true — never call plugin release. */
    if (m_ctx != nullptr) {
        CudaLib::release(m_ctx);
    }
}


size_t xmrig::CudaDevice::freeMemSize() const
{
#   ifdef XMRIG_FEATURE_CUDA_VERUS
    if (m_nativeRuntime) {
        size_t free_b = 0;
        size_t total  = 0;
        if (cudaSetDevice(static_cast<int>(m_index)) != cudaSuccess) {
            return 0;
        }
        if (cudaMemGetInfo(&free_b, &total) != cudaSuccess) {
            return 0;
        }
        return free_b;
    }
#   endif
    if (!m_ctx) {
        return 0;
    }

    return CudaLib::deviceUlong(m_ctx, CudaLib::DeviceMemoryFree);
}


size_t xmrig::CudaDevice::globalMemSize() const
{
#   ifdef XMRIG_FEATURE_CUDA_VERUS
    if (m_nativeRuntime) {
        return m_nativeTotalMem;
    }
#   endif
    if (!m_ctx) {
        return 0;
    }

    return CudaLib::deviceUlong(m_ctx, CudaLib::DeviceMemoryTotal);
}


uint32_t xmrig::CudaDevice::clock() const
{
#   ifdef XMRIG_FEATURE_CUDA_VERUS
    if (m_nativeRuntime) {
        return m_nativeClockKhz / 1000U;
    }
#   endif
    if (!m_ctx) {
        return 0;
    }

    return CudaLib::deviceUint(m_ctx, CudaLib::DeviceClockRate) / 1000;
}


uint32_t xmrig::CudaDevice::computeCapability(bool major) const
{
#   ifdef XMRIG_FEATURE_CUDA_VERUS
    if (m_nativeRuntime) {
        return major ? m_nativeArchMajor : m_nativeArchMinor;
    }
#   endif
    if (!m_ctx) {
        return 0;
    }

    return CudaLib::deviceUint(m_ctx, major ? CudaLib::DeviceArchMajor : CudaLib::DeviceArchMinor);
}


uint32_t xmrig::CudaDevice::memoryClock() const
{
#   ifdef XMRIG_FEATURE_CUDA_VERUS
    if (m_nativeRuntime) {
        return m_nativeMemClockKhz / 1000U;
    }
#   endif
    if (!m_ctx) {
        return 0;
    }

    return CudaLib::deviceUint(m_ctx, CudaLib::DeviceMemoryClockRate) / 1000;
}


uint32_t xmrig::CudaDevice::smx() const
{
#   ifdef XMRIG_FEATURE_CUDA_VERUS
    if (m_nativeRuntime) {
        return m_nativeSmx;
    }
#   endif
    if (!m_ctx) {
        return 0;
    }

    return CudaLib::deviceUint(m_ctx, CudaLib::DeviceSmx);
}


void xmrig::CudaDevice::generate(const Algorithm &algorithm, CudaThreads &threads) const
{
#   ifdef XMRIG_ALGO_VERUSHASH
    if (algorithm.family() == Algorithm::VERUSHASH_FAMILY) {
        if (!isValid()) {
            return;
        }

        threads.add(CudaThread(m_index, 1, 256, -1));

        return;
    }
#   endif

    if (!m_ctx || !CudaLib::deviceInfo(m_ctx, -1, -1, algorithm)) {
        return;
    }

    threads.add(CudaThread(m_index, m_ctx));
}


#ifdef XMRIG_FEATURE_API
void xmrig::CudaDevice::toJSON(rapidjson::Value &out, rapidjson::Document &doc) const
{
    using namespace rapidjson;
    auto &allocator = doc.GetAllocator();

    out.AddMember("name",           name().toJSON(doc), allocator);
    out.AddMember("bus_id",         topology().toString().toJSON(doc), allocator);
    out.AddMember("smx",            smx(), allocator);
    out.AddMember("arch",           arch(), allocator);
    out.AddMember("global_mem",     static_cast<uint64_t>(globalMemSize()), allocator);
    out.AddMember("clock",          clock(), allocator);
    out.AddMember("memory_clock",   memoryClock(), allocator);

#   ifdef XMRIG_FEATURE_NVML
    if (m_nvmlDevice) {
        auto data = NvmlLib::health(m_nvmlDevice);

        Value health(kObjectType);
        health.AddMember("temperature", data.temperature, allocator);
        health.AddMember("power",       data.power, allocator);
        health.AddMember("clock",       data.clock, allocator);
        health.AddMember("mem_clock",   data.memClock, allocator);

        Value fanSpeed(kArrayType);
        for (auto speed : data.fanSpeed) {
            fanSpeed.PushBack(speed, allocator);
        }
        health.AddMember("fan_speed", fanSpeed, allocator);

        out.AddMember("health", health, allocator);
    }
#   endif
}
#endif
