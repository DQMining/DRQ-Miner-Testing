/* DRQ Miner — terminal mining animation (-g/--graphic). */

#ifndef XMRIG_DRQGRAPHIC_H
#define XMRIG_DRQGRAPHIC_H


namespace xmrig {


class DrqGraphic
{
public:
    static void start();
    static void stop();

    static bool enabled();

    static void onNewJob();
    static void onShareHit(bool accepted);
    static void onBlockFound();

    /** Called from DaemonClient / SelfSelectClient on successful block submit. */
    static void notifyBlockSubmitSuccess();
};


} // namespace xmrig


#endif
