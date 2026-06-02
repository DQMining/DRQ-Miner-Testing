#pragma once

#include <cstdint>

inline unsigned char reverse8(unsigned char b)
{
    return static_cast<unsigned char>((static_cast<uint64_t>(b) * 0x0202020202ULL & 0x010884422010ULL) % 1023);
}
