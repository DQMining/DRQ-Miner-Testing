#ifndef XMRIG_ASTROBWT_WOLF_WORKER_H
#define XMRIG_ASTROBWT_WOLF_WORKER_H

#include <cstdint>

#include "crypto/astrobwt/hash_utils.h"

namespace xmrig {
namespace astrobwt {
namespace wolf {

constexpr int kMinPrefLen = 4;

struct TemplateMarker {
    uint8_t  p1;
    uint8_t  p2;
    uint16_t keySpotA;
    uint16_t keySpotB;
    uint16_t posData;
};

struct WolfWorker {
    uint8_t* sData = nullptr;

    TemplateMarker astroTemplate[277]{};
    int templateIdx = 0;
};

/** Per mining thread: binds sData to SPSA worker blob when available. */
WolfWorker& thread_worker();

} // namespace wolf
} // namespace astrobwt
} // namespace xmrig

#endif
