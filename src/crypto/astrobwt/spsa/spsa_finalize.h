#ifndef XMRIG_ASTROBWT_SPSA_FINALIZE_H
#define XMRIG_ASTROBWT_SPSA_FINALIZE_H

#include <cstdint>

namespace xmrig {
namespace astrobwt {
namespace spsa {

struct SpsaTemplateMarker {
    uint8_t  p1;
    uint8_t  p2;
    uint16_t keySpotA;
    uint16_t keySpotB;
    uint16_t posData;
};

bool available();
void init();

/** Persistent per-thread wolf/SPSA blob (ASTRO_SCRATCH_SIZE). Null when SPSA is disabled. */
uint8_t* worker_sdata();

bool finalize(const uint8_t* data, int data_len, uint8_t* output_hash);
bool finalize_with_templates(const uint8_t* data,
                             int data_len,
                             const SpsaTemplateMarker* templates,
                             int template_count,
                             uint8_t* output_hash);

} // namespace spsa
} // namespace astrobwt
} // namespace xmrig

#endif
