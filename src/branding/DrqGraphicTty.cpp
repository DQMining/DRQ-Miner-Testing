/* DRQ Miner — ANSI alt-screen ASCII animation (SSH / Windows console). */

#include "base/io/log/Log.h"
#include "base/kernel/Platform.h"

#include "res/graphic/tty_frames.inc.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <thread>

#ifdef XMRIG_OS_WIN
#   include <io.h>
#else
#   include <unistd.h>
#endif


namespace xmrig {
namespace {


static std::atomic<bool> g_run { false };
static std::atomic<bool> g_newJob { false };
static std::atomic<int>  g_shareFlash { 0 };
static std::atomic<int>  g_blockFlash { 0 };
static std::thread       g_thread;


static bool stdoutIsTty()
{
#ifdef XMRIG_OS_WIN
    return _isatty(_fileno(stdout)) != 0;
#else
    return isatty(fileno(stdout)) != 0;
#endif
}


static void terminalBell(int count)
{
    for (int i = 0; i < count; ++i) {
        fputc('\a', stdout);
    }
    fflush(stdout);
}


static void writeRaw(const char *s)
{
    fputs(s, stdout);
}


static void enterAltScreen()
{
    writeRaw("\033[?1049h\033[?25l\033[H\033[2J");
    fflush(stdout);
}


static void leaveAltScreen()
{
    writeRaw("\033[?1049l\033[?25h");
    fflush(stdout);
}


static void renderScene(size_t minerFrame, size_t coinStage, bool showJackpot)
{
    writeRaw("\033[H");

    if (showJackpot) {
        writeRaw("\033[1;33m");
        writeRaw(drq_graphic::kJackpotBanner);
        writeRaw("\033[0m\n\n");
    }

    writeRaw("\033[1;36m  DRQ MINER — GRAPHIC MODE\033[0m\n\n");

    writeRaw(drq_graphic::kManager);
    writeRaw("\n\n");

    writeRaw(drq_graphic::kBoulder);
    writeRaw("\n\n");

    if (coinStage < drq_graphic::kCoinStageCount) {
        writeRaw("\033[1;33m");
        writeRaw(drq_graphic::kCoinStages[coinStage]);
        writeRaw("\033[0m\n\n");
    }

    writeRaw("\033[1;32m");
    writeRaw(drq_graphic::kMinerFrames[minerFrame % drq_graphic::kMinerFrameCount]);
    writeRaw("\033[0m\n");

    fflush(stdout);
}


static void animationLoop()
{
    size_t minerFrame = 0;
    size_t coinStage  = 0;
    int blockTicks    = 0;

    while (g_run.load()) {
        if (g_newJob.exchange(false)) {
            coinStage = 0;
        }

        const int share = g_shareFlash.exchange(0);
        if (share > 0) {
            terminalBell(1);
            coinStage = drq_graphic::kCoinStageCount - 1;
        }

        const int block = g_blockFlash.exchange(0);
        if (block > 0) {
            terminalBell(3);
            blockTicks = 40;
        }

        const bool jackpot = blockTicks > 0;
        if (blockTicks > 0) {
            --blockTicks;
        }

        renderScene(minerFrame, coinStage, jackpot);

        ++minerFrame;
        if (coinStage + 1 < drq_graphic::kCoinStageCount && (minerFrame % 8) == 0) {
            ++coinStage;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
}


} // namespace


bool drqGraphicTtyStart()
{
    if (Log::isBackground() || !stdoutIsTty()) {
        return false;
    }

#ifdef XMRIG_OS_WIN
    Platform::enableConsoleColors();
#endif

    g_run.store(true);
    enterAltScreen();
    g_thread = std::thread(animationLoop);

    return true;
}


void drqGraphicTtyStop()
{
    if (!g_run.exchange(false)) {
        return;
    }

    if (g_thread.joinable()) {
        g_thread.join();
    }

    leaveAltScreen();
}


void drqGraphicTtyNewJob()
{
    g_newJob.store(true);
}


void drqGraphicTtyShareHit(bool /*accepted*/)
{
    g_shareFlash.store(1);
}


void drqGraphicTtyBlockFound()
{
    g_blockFlash.store(1);
}


} // namespace xmrig
