/* DRQ Miner — CUDA VerusHash: native GPU kernel when built with CUDA toolkit (monkins1010/ccminer). */

#ifndef XMRIG_CUDAVERUSHOSTRUNNER_H
#define XMRIG_CUDAVERUSHOSTRUNNER_H


#include "backend/cuda/interfaces/ICudaRunner.h"
#include "backend/cuda/CudaLaunchData.h"
#include "base/net/stratum/Job.h"


#include <cstdint>


namespace xmrig {


class CudaVerusHostRunner : public ICudaRunner
{
public:
    CudaVerusHostRunner(size_t id, const CudaLaunchData &data);
    ~CudaVerusHostRunner() override;

    size_t intensity() const override;
    size_t roundSize() const override          { return m_intensity; }
    size_t processedHashes() const override    { return m_intensity; }
    bool init() override;
    bool run(uint32_t startNonce, uint32_t *rescount, uint32_t *resnonce) override;
    bool set(const Job &job, uint8_t *blob) override;
    void jobEarlyNotification(const Job&) override {}

private:
    const CudaLaunchData &m_data;
    const uint32_t m_intensity;
    const int m_thrId;
    Job m_job{};
    uint8_t *m_blob = nullptr;

#   ifdef XMRIG_FEATURE_CUDA_VERUS
    bool m_cudaInit   = false;
    bool m_useGpu     = false;
    bool m_gpuJobOk   = false;
    uint8_t m_version = 0;
    alignas(16) uint32_t m_ptarget[8]{};
#   endif
};


} // namespace xmrig


#endif
