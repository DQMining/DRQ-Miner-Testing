#if defined(__aarch64__) && defined(XMRIG_ASTRO_AARCH64_ENABLED)

#include "crypto/astrobwt/AstroBWT.h"
#include "crypto/astrobwt/aarch64/astro_aarch64.h"
#include "crypto/astrobwt/sha256_utils.h"
#include "crypto/astrobwt/spsa/spsa_finalize.h"

#include <fnv1a.h>

#include <openssl/rc4.h>

#include <cstring>

extern "C" void* xmrig_astro_worker_data();

namespace xmrig {
namespace astrobwt {

bool astrobwt_dero_v3_aarch64(const void* input_data, uint32_t input_size, ScratchData* scratch, uint8_t* output_hash)
{
    (void)scratch;

    if (!spsa::available()) {
        return false;
    }

    spsa::init();

    void* ctx = xmrig_astro_worker_data();
    if (!ctx) {
        return false;
    }

    auto& worker = *static_cast<workerData*>(ctx);

    uint8_t scratch_buf[384] = {0};

    sha256_auto(static_cast<const uint8_t*>(input_data), input_size, &scratch_buf[320]);

    worker.salsa20.setKey(&scratch_buf[320]);
    worker.salsa20.setIv(&scratch_buf[256]);
    worker.salsa20.processBytes(worker.salsaInput, scratch_buf, 256);

    RC4_set_key(&worker.key[0], 256, scratch_buf);
    RC4(&worker.key[0], 256, scratch_buf, scratch_buf);

    std::memcpy(worker.sData, scratch_buf, 256);

    worker.lhash = hash_64_fnv1a_256(scratch_buf);
    worker.prev_lhash = worker.lhash;
    worker.tries[0] = 0;
    worker.isSame = false;
    worker.templateIdx = 0;

    branchComputeCPU_aarch64(worker, false, 0);

    if (worker.data_len > MAX_LENGTH) {
        worker.data_len = MAX_LENGTH;
    }

    const auto* templates = reinterpret_cast<const spsa::SpsaTemplateMarker*>(worker.astroTemplate);
    if (spsa::finalize_with_templates(worker.sData,
                                      static_cast<int>(worker.data_len),
                                      templates,
                                      worker.templateIdx,
                                      output_hash)) {
        return true;
    }

    return spsa::finalize(worker.sData, static_cast<int>(worker.data_len), output_hash);
}

} // namespace astrobwt
} // namespace xmrig

#endif
