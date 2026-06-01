/* DRQ Miner — OpenCL VerusHash (AMDVerusCoin / monkins1010 kernel). */

#ifndef XMRIG_OCLVERUSHASHRUNNER_H
#define XMRIG_OCLVERUSHASHRUNNER_H


#include "backend/opencl/runners/OclBaseRunner.h"
#include "backend/opencl/wrappers/OclDevice.h"
#include "base/net/stratum/Job.h"


namespace xmrig {


class OclVerusHashRunner : public OclBaseRunner
{
public:
    OclVerusHashRunner(size_t index, const OclLaunchData &data);
    ~OclVerusHashRunner() override;

    void build() override;
    void init() override;
    uint32_t processedHashes() const override;

    static uint32_t verusSlotCount(uint32_t intensity);
    static uint32_t verusIntensityForDevice(const OclDevice &device);

protected:
    void run(uint32_t nonce, uint32_t nonce_offset, uint32_t *hashOutput) override;
    void set(const Job &job, uint8_t *blob) override;

private:

    Job m_job{};
    uint8_t *m_blob = nullptr;

    cl_kernel m_kernel = nullptr;

    cl_mem m_startNonce = nullptr;
    cl_mem m_blockhash  = nullptr;
    cl_mem m_vkey       = nullptr;
    cl_mem m_target     = nullptr;
    cl_mem m_resNonce   = nullptr;
    cl_mem m_longKeys   = nullptr;

    uint32_t m_slots      = 0;
    uint8_t  m_version    = 0;
    uint8_t  m_ptarget[32]{};

    bool m_gpuReady  = false;
    bool m_useGpu    = false;
    bool m_gpuJobOk  = false;
};


} // namespace xmrig


#endif
