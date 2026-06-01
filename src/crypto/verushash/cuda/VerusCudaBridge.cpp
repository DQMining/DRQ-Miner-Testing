/* Host-side Verus GPU preimage helpers (Haraka portable) — aligned with monkins1010/ccminer verus2.2gpu verusscan.cpp */

#include "crypto/verushash/cuda/VerusCudaBridge.h"

#include <algorithm>
#include <cstring>

/* haraka_portable.c is C; haraka_portable.h does not use extern "C", so C++ TUs must declare C linkage. */
extern "C" {
void load_constants_port(void);
void haraka256_port(unsigned char *out, const unsigned char *in);
void haraka512_port(unsigned char *out, const unsigned char *in);
}

static constexpr int kVerusKeySize = 8832;

void verus_cuda_VerusHashHalf(void *result64, const unsigned char *data, int len)
{
    alignas(32) unsigned char buf1[64] = { 0 };
    alignas(32) unsigned char buf2[64]  = { 0 };
    unsigned char *curBuf  = buf1;
    unsigned char *result  = buf2;
    size_t curPos          = 0;

    std::fill(buf1, buf1 + sizeof(buf1), 0);

    load_constants_port();

    for (int pos = 0; pos < len;) {
        const int room = 32 - static_cast<int>(curPos);

        if (len - pos >= room) {
            memcpy(curBuf + 32 + curPos, data + pos, room);
            haraka512_port(result, curBuf);
            unsigned char *tmp = curBuf;
            curBuf              = result;
            result              = tmp;
            pos += room;
            curPos = 0;
        }
        else {
            memcpy(curBuf + 32 + curPos, data + pos, len - pos);
            curPos += len - pos;
            pos = len;
        }
    }

    memcpy(curBuf + 47, curBuf, 16);
    memcpy(curBuf + 63, curBuf, 1);
    memcpy(result64, curBuf, 64);
}

void verus_cuda_GenNewCLKey(const unsigned char *seed32, void *key8832)
{
    auto *pkey               = static_cast<unsigned char *>(key8832);
    const unsigned char *psrc = seed32;
    const int n256blks   = kVerusKeySize >> 5;
    const int nbytesExtra = kVerusKeySize & 0x1f;

    for (int i = 0; i < n256blks; i++) {
        haraka256_port(pkey, psrc);
        psrc = pkey;
        pkey += 32;
    }
    if (nbytesExtra != 0) {
        unsigned char buf[32];
        haraka256_port(buf, psrc);
        memcpy(pkey, buf, nbytesExtra);
    }
}
