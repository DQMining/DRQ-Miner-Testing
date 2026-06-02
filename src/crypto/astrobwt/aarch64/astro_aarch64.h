#ifndef XMRIG_ASTROBWT_AARCH64_H
#define XMRIG_ASTROBWT_AARCH64_H

#if defined(__aarch64__)

#include <arm_neon.h>

#include "astroworker.h"

constexpr int sus_op = 1;

void branchComputeCPU_aarch64(workerData& worker, bool isTest, int wIndex);

#endif

#endif
