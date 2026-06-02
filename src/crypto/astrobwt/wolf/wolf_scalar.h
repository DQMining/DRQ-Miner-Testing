#ifndef XMRIG_ASTROBWT_WOLF_SCALAR_H
#define XMRIG_ASTROBWT_WOLF_SCALAR_H

#include "crypto/astrobwt/hash_utils.h"
#include "crypto/astrobwt/wolf/wolf_tables.h"

#include <cstdint>
#include <cstring>

namespace xmrig {
namespace astrobwt {
namespace wolf {

inline uint8_t apply_wolf_insn(uint8_t val, uint8_t pos2val, uint8_t insn)
{
    switch (insn) {
    case 0:  return static_cast<uint8_t>(val + val);
    case 1:  return static_cast<uint8_t>(val - (val ^ 97));
    case 2:  return static_cast<uint8_t>(val * val);
    case 3:  return static_cast<uint8_t>(val ^ pos2val);
    case 4:  return static_cast<uint8_t>(~val);
    case 5:  return static_cast<uint8_t>(val & pos2val);
    case 6:  return static_cast<uint8_t>(val << (val & 3));
    case 7:  return static_cast<uint8_t>(val >> (val & 3));
    case 8:
        val = static_cast<uint8_t>(((val & 0xAA) >> 1) | ((val & 0x55) << 1));
        val = static_cast<uint8_t>(((val & 0xCC) >> 2) | ((val & 0x33) << 2));
        return static_cast<uint8_t>(((val & 0xF0) >> 4) | ((val & 0x0F) << 4));
    case 9:  return static_cast<uint8_t>(val ^ popcount8(val));
    case 10: return rotl8(val, val);
    case 11: return rotl8(val, 1);
    case 12: return static_cast<uint8_t>(val ^ rotl8(val, 2));
    case 13: return rotl8(val, 3);
    case 14: return static_cast<uint8_t>(val ^ rotl8(val, 4));
    case 15: return rotl8(val, 5);
    default: return val;
    }
}

inline void wolf_permute_inplace_scalar(uint8_t* s, uint16_t op, uint8_t pos1, uint8_t pos2)
{
    uint8_t old[32];
    uint8_t data[32];
    std::memcpy(old, &s[pos1], 32);

    const uint32_t opcode = CodeLUT_16[op];
    std::memcpy(data, old, 32);

    for (int inst = 3; inst >= 0; --inst) {
        const uint8_t insn = static_cast<uint8_t>((opcode >> (inst << 2)) & 0xF);
        for (int j = 0; j < 32; ++j) {
            data[j] = apply_wolf_insn(data[j], s[pos2], insn);
        }
    }

    const int blend = pos2 - pos1;
    for (int j = 0; j < 32; ++j) {
        s[pos1 + j] = (j < blend) ? data[j] : old[j];
    }
}

} // namespace wolf
} // namespace astrobwt
} // namespace xmrig

#endif
