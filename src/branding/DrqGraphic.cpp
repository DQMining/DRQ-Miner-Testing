/* DRQ Miner — terminal mining animation facade. */

#include "branding/DrqGraphic.h"

#include "base/io/log/Log.h"


namespace xmrig {


bool drqGraphicTtyStart();
void drqGraphicTtyStop();
void drqGraphicTtyNewJob();
void drqGraphicTtyShareHit(bool accepted);
void drqGraphicTtyBlockFound();


namespace {


bool g_enabled = false;


} // namespace


void DrqGraphic::start()
{
    if (Log::isBackground() || g_enabled) {
        return;
    }

    g_enabled = drqGraphicTtyStart();
}


void DrqGraphic::stop()
{
    if (!g_enabled) {
        return;
    }

    drqGraphicTtyStop();
    g_enabled = false;
}


bool DrqGraphic::enabled()
{
    return g_enabled;
}


void DrqGraphic::onNewJob()
{
    if (g_enabled) {
        drqGraphicTtyNewJob();
    }
}


void DrqGraphic::onShareHit(bool accepted)
{
    if (g_enabled) {
        drqGraphicTtyShareHit(accepted);
    }
}


void DrqGraphic::onBlockFound()
{
    if (g_enabled) {
        drqGraphicTtyBlockFound();
    }
}


void DrqGraphic::notifyBlockSubmitSuccess()
{
    onBlockFound();
}


} // namespace xmrig
