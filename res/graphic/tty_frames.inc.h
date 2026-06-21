/* DRQ Miner — ASCII scene frames for -g/--graphic TTY animation. */

#ifndef XMRIG_TTY_FRAMES_INC_H
#define XMRIG_TTY_FRAMES_INC_H


namespace xmrig {
namespace drq_graphic {


static const char *const kMinerFrames[] = {
    "       \\o/  pickaxe\n"
    "        |\\\n"
    "       / \\",

    "       _O_  pickaxe\n"
    "      /| \\\n"
    "       / \\",

    "       o/  pickaxe\n"
    "       /| \n"
    "      / \\ ",

    "       O   pickaxe\n"
    "      /|\\ \n"
    "      / \\ ",
};

static constexpr size_t kMinerFrameCount = sizeof(kMinerFrames) / sizeof(kMinerFrames[0]);


static const char *const kBoulder =
    "     .-------.\n"
    "    /  BOULDER \\\n"
    "   |   #######  |\n"
    "    \\___________/";


static const char *const kCoinStages[] = {
    "        .\n"
    "      (coin)",

    "       ..\n"
    "     ((coin))",

    "      ...$\n"
    "    (( COIN ))",
};

static constexpr size_t kCoinStageCount = sizeof(kCoinStages) / sizeof(kCoinStages[0]);


static const char *const kManager =
    "   [ MANAGER ]\n"
    "    watching\n"
    "      o_o";


static const char *const kJackpotBanner =
    "  * * *  J A C K P O T  * * *\n"
    "   BLOCK FOUND — DRQ MINER\n"
    "  * * *  J A C K P O T  * * *";


} // namespace drq_graphic
} // namespace xmrig


#endif
