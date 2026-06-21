#include "crypto/nm/NmShared.h"
#include "crypto/common/VirtualMemory.h"

#include <cstring>
#include <map>
#include <mutex>

#include "crypto/nm/nm_fast.h"
#include "crypto/nm/nm_params.h"

namespace xmrig {

struct NmNodeState {
    VirtualMemory *cur  = nullptr;
    VirtualMemory *prev = nullptr;
    nm_epoch       epoch{};
    uint8_t        seed[32] = { 0 };
    bool           valid = false;
};

static std::mutex g_nmMutex;
static std::map<uint32_t, NmNodeState> g_nmNodes;

const nm_epoch *NmShared::epoch(const uint8_t seed[32], uint32_t node, bool hugePages)
{
    std::lock_guard<std::mutex> lock(g_nmMutex);

    NmNodeState &n = g_nmNodes[node];

    if (!n.valid || memcmp(n.seed, seed, 32) != 0) {
        auto *vm = new VirtualMemory(NM_DATASET_BYTES, hugePages, false, false, node, VirtualMemory::kDefaultHugePageSize);

        nm_fast_epoch_init(&n.epoch, seed);
        nm_fast_build_dataset(&n.epoch, reinterpret_cast<uint64_t *>(vm->scratchpad()));
        nm_fast_set_dataset(&n.epoch, reinterpret_cast<uint64_t *>(vm->scratchpad()));

        delete n.prev;
        n.prev = n.cur;
        n.cur  = vm;
        memcpy(n.seed, seed, 32);
        n.valid = true;
    }

    return &n.epoch;
}

const uint64_t *NmShared::dataset(const uint8_t seed[32], uint32_t node, bool hugePages, uint8_t out_key[16])
{
    const nm_epoch *e = epoch(seed, node, hugePages);
    if (!e) {
        return nullptr;
    }

    memcpy(out_key, e->params.dataset_key, 16);
    return e->dataset;
}

}
