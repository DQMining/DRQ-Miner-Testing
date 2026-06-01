/*
 * Based on public domain code available at: http://cr.yp.to/snuffle.html
 *
 * This therefore is public domain.
 */

#ifndef ZT_SALSA20_HPP
#define ZT_SALSA20_HPP

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

namespace ZeroTier {

class Salsa20
{
public:
    Salsa20(const void *key, const void *iv)
    {
        init(key, iv);
    }

    void init(const void *key, const void *iv);
    void XORKeyStream(void *out, unsigned int bytes);

private:
    union {
        __m128i v[4];
        uint32_t i[16];
    } _state;
};

} // namespace ZeroTier

#endif
