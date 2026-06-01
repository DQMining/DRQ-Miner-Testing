#ifndef XMRIG_ASTROBWT_SUFFIX_H
#define XMRIG_ASTROBWT_SUFFIX_H

#include <cstdint>

namespace xmrig {
namespace astrobwt {

struct ScratchData;

namespace spsa {
struct SpsaTemplateMarker;
}

void suffix_hash(uint8_t* data,
                 int32_t data_len,
                 ScratchData* scratch,
                 uint8_t* output_hash,
                 const spsa::SpsaTemplateMarker* templates = nullptr,
                 int template_count = 0);

} // namespace astrobwt
} // namespace xmrig

#endif
