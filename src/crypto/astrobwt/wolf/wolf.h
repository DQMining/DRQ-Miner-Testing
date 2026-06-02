#ifndef XMRIG_ASTROBWT_WOLF_H
#define XMRIG_ASTROBWT_WOLF_H

#include <cstdint>

namespace xmrig {
namespace astrobwt {

struct ScratchData;

namespace wolf {

#if defined(__AVX2__) || (defined(_MSC_VER) && defined(__AVX2__)) || defined(XMRIG_ARM)
#   define XMRIG_WOLF_ENABLED 1
#endif

void init();
bool available();
bool astrobwt_dero_v3_wolf(const void* input_data, uint32_t input_size, ScratchData* scratch, uint8_t* output_hash);

} // namespace wolf
} // namespace astrobwt
} // namespace xmrig

#endif
