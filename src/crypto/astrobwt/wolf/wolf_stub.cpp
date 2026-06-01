#include "crypto/astrobwt/wolf/wolf.h"

namespace xmrig {
namespace astrobwt {
namespace wolf {

#if !defined(__AVX2__) && !(defined(_MSC_VER) && defined(__AVX2__))

void init() {}

bool available()
{
    return false;
}

bool astrobwt_dero_v3_wolf(const void*, uint32_t, ScratchData*, uint8_t*)
{
    return false;
}

#endif

} // namespace wolf
} // namespace astrobwt
} // namespace xmrig
