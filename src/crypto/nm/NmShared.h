#ifndef XMRIG_NM_SHARED_H
#define XMRIG_NM_SHARED_H

#include <cstdint>

#include "crypto/nm/nm_fast.h"

namespace xmrig {

class NmShared
{
public:

    static const nm_epoch *epoch(const uint8_t seed[32], uint32_t node, bool hugePages);
    static const uint64_t *dataset(const uint8_t seed[32], uint32_t node, bool hugePages, uint8_t out_key[16]);
};

}

#endif
