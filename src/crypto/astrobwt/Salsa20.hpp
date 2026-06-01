/*
 * Based on public domain code available at: http://cr.yp.to/snuffle.html
 *
 * This therefore is public domain.
 */

#ifndef ZT_SALSA20_HPP
#define ZT_SALSA20_HPP

#include <cstdint>

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
    uint32_t _state[16];
};

} // namespace ZeroTier

#endif
