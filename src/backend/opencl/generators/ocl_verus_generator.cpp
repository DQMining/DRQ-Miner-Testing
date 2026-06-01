/* DRQ Miner — auto OpenCL threads for VerusHash (host hash path). */

#include <algorithm>

#include "backend/opencl/OclThreads.h"
#include "backend/opencl/runners/OclVerusHashRunner.h"
#include "backend/opencl/wrappers/OclDevice.h"
#include "base/crypto/Algorithm.h"


namespace xmrig {


bool ocl_verus_generator(const OclDevice &device, const Algorithm &algorithm, OclThreads &threads)
{
    if (algorithm.family() != Algorithm::VERUSHASH_FAMILY) {
        return false;
    }

    const uint32_t intensity = OclVerusHashRunner::verusIntensityForDevice(device);
    // Full ctor: KawPow slim builds disable the 4-arg OclThread constructor.
    threads.add(OclThread(device.index(), intensity, 128, 0, 2, 1, 8));

    return true;
}


} // namespace xmrig
