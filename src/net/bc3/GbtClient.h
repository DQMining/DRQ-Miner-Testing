/* BitcoinIII (BC3) — getblocktemplate HTTP polling client (Phase 2 skeleton). */

#ifndef XMRIG_GBTCLIENT_H
#define XMRIG_GBTCLIENT_H


#include "base/net/stratum/Pool.h"


namespace xmrig {


class GbtClient
{
public:
    static bool isBc3Pool(const Pool &pool);
    static void bootstrap(const Pool &pool);

    GbtClient(const Pool &pool);
    ~GbtClient();

    void poll();

    static const char *algoPlaceholder();

private:
    void requestBlockTemplate();

    Pool m_pool;
    bool m_ready = false;
};


} // namespace xmrig


#endif
