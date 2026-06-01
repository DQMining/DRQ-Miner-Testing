#ifndef XMRIG_ASTROBWT_WOLF_TABLES_H
#define XMRIG_ASTROBWT_WOLF_TABLES_H

#include <cstdint>

namespace xmrig {
namespace astrobwt {
namespace wolf {

void init_wolf_lut();

uint8_t wolf_branch(uint8_t val, uint8_t pos2val, uint32_t opcode);

extern alignas(32) uint32_t CodeLUT[257];
extern alignas(32) uint16_t CodeLUT_16[257];

} // namespace wolf
} // namespace astrobwt
} // namespace xmrig

#endif
