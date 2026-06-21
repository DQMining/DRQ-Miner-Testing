/* DRQ Miner — launch banner (pixel logo from res/drq_logo_forum.txt). */

#include "branding/DrqBanner.h"
#include "branding/DrqBannerLogo.inc.h"

#include "base/io/log/Log.h"
#include "base/kernel/Platform.h"
#include "version.h"

#include <cstdio>

#ifdef XMRIG_OS_WIN
#   include <windows.h>
#endif


namespace xmrig {


namespace {


static void enableUtf8Console()
{
#ifdef XMRIG_OS_WIN
    SetConsoleOutputCP(CP_UTF8);
#endif
}


static void printLogo(bool colors)
{
    fputs("\n", stdout);

    for (size_t i = 0; i < kDrqLogoRows; ++i) {
        fputs(colors ? kDrqLogoColor[i] : kDrqLogoPlain[i], stdout);
        fputs("\n", stdout);
    }

    fputs("\n", stdout);

    if (colors) {
        fputs(RED_BOLD_S "    * * *  Dr. Q  —  DRQ MINER  —  AstroBWT v3  |  VerusHash  * * *\n" CLEAR, stdout);
        fputs(CYAN_BOLD_S "    Stable: " APP_STABLE_FORMAT "  |  Beta: " APP_BETA_FORMAT "\n" CLEAR, stdout);
    }
    else {
        fputs("    * * *  Dr. Q  —  DRQ MINER  —  AstroBWT v3  |  VerusHash  * * *\n", stdout);
        fputs("    Stable: " APP_STABLE_FORMAT "  |  Beta: " APP_BETA_FORMAT "\n", stdout);
    }

    fputs("\n", stdout);
}


} // namespace


void DrqBanner::print()
{
    if (Log::isBackground()) {
        return;
    }

    enableUtf8Console();

    /* Banner uses truecolor directly; do not depend on Log/ConsoleLog init (-V path). */
    const bool colors = Platform::enableConsoleColors();
    printLogo(colors);
    fflush(stdout);
}


} // namespace xmrig
