#include <atomic>
#include <string>

// Symbols referenced by the prebuilt AstroSPSA (TNN) library.
bool ABORT_MINER = false;
std::string devWallet;
int devFee = 0;

#if defined(__aarch64__)
const char* tnnTargetArch = "armv8-a+crypto";
#else
const char* tnnTargetArch = "x86-64-v3";
#endif
