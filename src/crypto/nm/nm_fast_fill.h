#ifndef NM_FAST_FILL_H
#define NM_FAST_FILL_H

#include "nm_fast.h"

#ifdef __cplusplus
extern "C" {
#endif

/* VAES-256 scratch fill (MSVC: compiled separately with clang-cl -mvaes). */
void nm_fast_fill_vaes256(const nm_epoch *e, nm_lane *l, const uint8_t seed[32]);

#ifdef __cplusplus
}
#endif

#endif
