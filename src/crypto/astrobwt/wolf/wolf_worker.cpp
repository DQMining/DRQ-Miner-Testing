#include "crypto/astrobwt/wolf/wolf_worker.h"

#include "crypto/astrobwt/spsa/spsa_finalize.h"

namespace xmrig {
namespace astrobwt {
namespace wolf {

WolfWorker& thread_worker()
{
    static thread_local WolfWorker worker;
    static thread_local bool sdata_bound = false;

    if (!sdata_bound) {
        if (uint8_t* blob = spsa::worker_sdata()) {
            worker.sData = blob;
        }
        sdata_bound = true;
    }

    return worker;
}

} // namespace wolf
} // namespace astrobwt
} // namespace xmrig
