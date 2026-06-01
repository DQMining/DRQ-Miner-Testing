// Copyright 2019 The Go Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
//
// Port of SAIS (Suffix Array by Induced Sorting) from Go to C++.
// Original: Go suffixarray/sais.go and sais2.go
//
// See Ge Nong, Sen Zhang, and Wai Hong Chen,
// "Two Efficient Algorithms for Linear Time Suffix Array Construction".

#ifndef XMRIG_ASTROBWT_SAIS_H
#define XMRIG_ASTROBWT_SAIS_H

#include <cstdint>
#include <cstring>
#include <vector>


namespace xmrig {
namespace astrobwt {

//------------------------------------------------------------------------------
// freq_8_32: compute character frequencies for byte text
//------------------------------------------------------------------------------
static inline void freq_8_32(const uint8_t* text, int32_t n,
                             int32_t* freq, int32_t* bucket)
{
    if (freq != nullptr && freq[0] >= 0) {
        return;
    }
    int32_t* f = (freq != nullptr) ? freq : bucket;
    for (int32_t i = 0; i < 256; i++) {
        f[i] = 0;
    }
    for (int32_t i = 0; i < n; i++) {
        f[static_cast<int32_t>(text[i])]++;
    }
}

//------------------------------------------------------------------------------
// bucketMin_8_32 / bucketMax_8_32
//------------------------------------------------------------------------------
static inline void bucketMin_8_32(const uint8_t* text, int32_t n,
                                 int32_t* freq, int32_t* bucket)
{
    freq_8_32(text, n, freq, bucket);
    int32_t* f = (freq != nullptr) ? freq : bucket;
    int32_t total = 0;
    for (int32_t i = 0; i < 256; i++) {
        bucket[i] = total;
        total += f[i];
    }
}

static inline void bucketMax_8_32(const uint8_t* text, int32_t n,
                                 int32_t* freq, int32_t* bucket)
{
    freq_8_32(text, n, freq, bucket);
    int32_t* f = (freq != nullptr) ? freq : bucket;
    int32_t total = 0;
    for (int32_t i = 0; i < 256; i++) {
        total += f[i];
        bucket[i] = total;
    }
}

//------------------------------------------------------------------------------
// placeLMS_8_32 - Go: c0, c1 = text[i], c0 => C++: c1 = c0; c0 = text[i];
//------------------------------------------------------------------------------
static inline int32_t placeLMS_8_32(const uint8_t* text, int32_t n,
                                    int32_t* sa, int32_t* freq, int32_t* bucket)
{
    bucketMax_8_32(text, n, freq, bucket);

    int32_t numLMS = 0;
    int32_t lastB = -1;

    uint8_t c0 = 0, c1 = 0;
    bool isTypeS = false;
    for (int32_t i = n - 1; i >= 0; i--) {
        c1 = c0;
        c0 = text[i];
        if (c0 < c1) {
            isTypeS = true;
        } else if (c0 > c1 && isTypeS) {
            isTypeS = false;
            int32_t b = bucket[static_cast<int32_t>(c1)] - 1;
            bucket[static_cast<int32_t>(c1)] = b;
            sa[b] = i + 1;
            lastB = b;
            numLMS++;
        }
    }

    if (numLMS > 1) {
        sa[lastB] = 0;
    }
    return numLMS;
}

//------------------------------------------------------------------------------
// induceSubL_8_32
//------------------------------------------------------------------------------
static inline void induceSubL_8_32(const uint8_t* text, int32_t n, int32_t* sa,
                                   int32_t* freq, int32_t* bucket)
{
    bucketMin_8_32(text, n, freq, bucket);

    int32_t k = n - 1;
    uint8_t c0 = text[k - 1];
    uint8_t c1 = text[k];
    if (c0 < c1) {
        k = -k;
    }

    uint8_t cB = c1;
    int32_t b = bucket[static_cast<int32_t>(cB)];
    sa[b] = k;
    b++;

    for (int32_t i = 0; i < n; i++) {
        int32_t j = sa[i];
        if (j == 0) continue;
        if (j < 0) {
            sa[i] = -j;
            continue;
        }
        sa[i] = 0;

        k = j - 1;
        c0 = text[k - 1];
        c1 = text[k];
        if (c0 < c1) {
            k = -k;
        }

        if (cB != c1) {
            bucket[static_cast<int32_t>(cB)] = b;
            cB = c1;
            b = bucket[static_cast<int32_t>(cB)];
        }
        sa[b] = k;
        b++;
    }
}

//------------------------------------------------------------------------------
// induceSubS_8_32
//------------------------------------------------------------------------------
static inline void induceSubS_8_32(const uint8_t* text, int32_t n, int32_t* sa,
                                   int32_t* freq, int32_t* bucket)
{
    bucketMax_8_32(text, n, freq, bucket);

    uint8_t cB = 0;
    int32_t b = bucket[cB];
    int32_t top = n;

    for (int32_t i = n - 1; i >= 0; i--) {
        int32_t j = sa[i];
        if (j == 0) continue;
        sa[i] = 0;
        if (j < 0) {
            top--;
            sa[top] = -j;
            continue;
        }

        int32_t k = j - 1;
        uint8_t c0 = text[k - 1];
        uint8_t c1 = text[k];
        if (c0 > c1) {
            k = -k;
        }

        if (cB != c1) {
            bucket[static_cast<int32_t>(cB)] = b;
            cB = c1;
            b = bucket[static_cast<int32_t>(cB)];
        }
        b--;
        sa[b] = k;
    }
}

//------------------------------------------------------------------------------
// length_8_32
//------------------------------------------------------------------------------
static inline void length_8_32(const uint8_t* text, int32_t n, int32_t* sa,
                               int32_t numLMS)
{
    int32_t end = 0;
    uint32_t cx = 0;

    uint8_t c0 = 0, c1 = 0;
    bool isTypeS = false;
    for (int32_t i = n - 1; i >= 0; i--) {
        c1 = c0;
        c0 = text[i];
        cx = (cx << 8) | static_cast<uint32_t>(c1 + 1);

        if (c0 < c1) {
            isTypeS = true;
        } else if (c0 > c1 && isTypeS) {
            isTypeS = false;

            int32_t j = i + 1;
            int32_t code;
            if (end == 0) {
                code = 0;
            } else {
                code = end - j;
                if (code <= 4 && (static_cast<uint32_t>(~cx) >= static_cast<uint32_t>(n))) {
                    code = static_cast<int32_t>(~cx);
                }
            }
            sa[j >> 1] = code;
            end = j + 1;
            cx = static_cast<uint32_t>(c1 + 1);
        }
    }
}

//------------------------------------------------------------------------------
// assignID_8_32
//------------------------------------------------------------------------------
static inline int32_t assignID_8_32(const uint8_t* text, int32_t n, int32_t* sa,
                                    int32_t numLMS)
{
    int32_t id = 0;
    int32_t lastLen = -1;
    int32_t lastPos = 0;
    int32_t* lmsPtr = sa + n - numLMS;

    for (int32_t idx = 0; idx < numLMS; idx++) {
        int32_t j = lmsPtr[idx];
        int32_t nVal = sa[j >> 1];
        bool isNew = true;

        if (nVal == lastLen) {
            if (static_cast<uint32_t>(nVal) >= static_cast<uint32_t>(n)) {
                isNew = false;
            } else {
                int32_t len = nVal;
                bool same = true;
                for (int32_t i = 0; i < len; i++) {
                    if (text[j + i] != text[lastPos + i]) {
                        same = false;
                        break;
                    }
                }
                if (same) isNew = false;
            }
        }

        if (isNew) {
            id++;
            lastPos = j;
            lastLen = nVal;
        }
        sa[j >> 1] = id;
    }
    return id;
}

//------------------------------------------------------------------------------
// map_32
//------------------------------------------------------------------------------
static inline void map_32(int32_t* sa, int32_t n, int32_t numLMS)
{
    int32_t w = n;
    for (int32_t i = n / 2; i >= 0; i--) {
        int32_t j = sa[i];
        if (j > 0) {
            w--;
            sa[w] = j - 1;
        }
    }
}

//------------------------------------------------------------------------------
// unmap_8_32 - LMS-substring iterator: c1 = c0; c0 = text[i];
//------------------------------------------------------------------------------
static inline void unmap_8_32(const uint8_t* text, int32_t n, int32_t* sa,
                              int32_t numLMS)
{
    int32_t* unmap = sa + n - numLMS;
    int32_t j = numLMS;

    uint8_t c0 = 0, c1 = 0;
    bool isTypeS = false;
    for (int32_t i = n - 1; i >= 0; i--) {
        c1 = c0;
        c0 = text[i];
        if (c0 < c1) {
            isTypeS = true;
        } else if (c0 > c1 && isTypeS) {
            isTypeS = false;
            j--;
            unmap[j] = i + 1;
        }
    }

    for (int32_t i = 0; i < numLMS; i++) {
        sa[i] = unmap[sa[i]];
    }
}

//------------------------------------------------------------------------------
// expand_8_32
//------------------------------------------------------------------------------
static inline void expand_8_32(const uint8_t* text, int32_t n, int32_t* freq,
                               int32_t* bucket, int32_t* sa, int32_t numLMS)
{
    bucketMax_8_32(text, n, freq, bucket);

    int32_t x = numLMS - 1;
    int32_t saX = sa[x];
    uint8_t c = text[saX];
    int32_t b = bucket[static_cast<int32_t>(c)] - 1;
    bucket[static_cast<int32_t>(c)] = b;

    for (int32_t i = n - 1; i >= 0; i--) {
        if (i != b) {
            sa[i] = 0;
            continue;
        }
        sa[i] = saX;
        if (x > 0) {
            x--;
            saX = sa[x];
            c = text[saX];
            b = bucket[static_cast<int32_t>(c)] - 1;
            bucket[static_cast<int32_t>(c)] = b;
        }
    }
}

//------------------------------------------------------------------------------
// induceL_8_32
//------------------------------------------------------------------------------
static inline void induceL_8_32(const uint8_t* text, int32_t n, int32_t* sa,
                               int32_t* freq, int32_t* bucket)
{
    bucketMin_8_32(text, n, freq, bucket);

    int32_t k = n - 1;
    uint8_t c0 = text[k - 1];
    uint8_t c1 = text[k];
    if (c0 < c1) {
        k = -k;
    }

    uint8_t cB = c1;
    int32_t b = bucket[static_cast<int32_t>(cB)];
    sa[b] = k;
    b++;

    for (int32_t i = 0; i < n; i++) {
        int32_t j = sa[i];
        if (j <= 0) continue;

        k = j - 1;
        c1 = text[k];
        if (k > 0) {
            c0 = text[k - 1];
            if (c0 < c1) k = -k;
        }

        if (cB != c1) {
            bucket[static_cast<int32_t>(cB)] = b;
            cB = c1;
            b = bucket[static_cast<int32_t>(cB)];
        }
        sa[b] = k;
        b++;
    }
}

//------------------------------------------------------------------------------
// induceS_8_32
//------------------------------------------------------------------------------
static inline void induceS_8_32(const uint8_t* text, int32_t n, int32_t* sa,
                                int32_t* freq, int32_t* bucket)
{
    bucketMax_8_32(text, n, freq, bucket);

    uint8_t cB = 0;
    int32_t b = bucket[cB];

    for (int32_t i = n - 1; i >= 0; i--) {
        int32_t j = sa[i];
        if (j >= 0) continue;

        j = -j;
        sa[i] = j;

        int32_t k = j - 1;
        uint8_t c1 = text[k];
        if (k > 0) {
            uint8_t c0 = text[k - 1];
            if (c0 <= c1) k = -k;
        }

        if (cB != c1) {
            bucket[static_cast<int32_t>(cB)] = b;
            cB = c1;
            b = bucket[static_cast<int32_t>(cB)];
        }
        b--;
        sa[b] = k;
    }
}

//==============================================================================
// sais_32 variant: operates on int32 text (for recursion)
//==============================================================================

static inline void freq_32(const int32_t* text, int32_t textLen, int32_t textMax,
                          int32_t* freq, int32_t* bucket)
{
    if (freq != nullptr && freq[0] >= 0) return;
    int32_t* f = (freq != nullptr) ? freq : bucket;
    for (int32_t i = 0; i < textMax; i++) f[i] = 0;
    for (int32_t i = 0; i < textLen; i++) f[text[i]]++;
}

static inline void bucketMin_32(const int32_t* text, int32_t textLen, int32_t textMax,
                               int32_t* freq, int32_t* bucket)
{
    freq_32(text, textLen, textMax, freq, bucket);
    int32_t* f = (freq != nullptr) ? freq : bucket;
    int32_t total = 0;
    for (int32_t i = 0; i < textMax; i++) {
        bucket[i] = total;
        total += f[i];
    }
}

static inline void bucketMax_32(const int32_t* text, int32_t textLen, int32_t textMax,
                               int32_t* freq, int32_t* bucket)
{
    freq_32(text, textLen, textMax, freq, bucket);
    int32_t* f = (freq != nullptr) ? freq : bucket;
    int32_t total = 0;
    for (int32_t i = 0; i < textMax; i++) {
        total += f[i];
        bucket[i] = total;
    }
}

static inline int32_t placeLMS_32(const int32_t* text, int32_t textLen,
                                  int32_t* sa, int32_t* freq, int32_t* bucket,
                                  int32_t textMax)
{
    bucketMax_32(text, textLen, textMax, freq, bucket);
    int32_t numLMS = 0;
    int32_t lastB = -1;

    int32_t c0 = 0, c1 = 0;
    bool isTypeS = false;
    for (int32_t i = textLen - 1; i >= 0; i--) {
        c1 = c0;
        c0 = text[i];
        if (c0 < c1) {
            isTypeS = true;
        } else if (c0 > c1 && isTypeS) {
            isTypeS = false;
            int32_t b = bucket[c1] - 1;
            bucket[c1] = b;
            sa[b] = i + 1;
            lastB = b;
            numLMS++;
        }
    }
    if (numLMS > 1) sa[lastB] = 0;
    return numLMS;
}

static inline void induceSubL_32(const int32_t* text, int32_t textLen, int32_t textMax,
                                 int32_t* sa, int32_t* freq, int32_t* bucket)
{
    bucketMin_32(text, textLen, textMax, freq, bucket);
    int32_t k = textLen - 1;
    int32_t c0 = text[k - 1];
    int32_t c1 = text[k];
    if (c0 < c1) k = -k;

    int32_t cB = c1;
    int32_t b = bucket[cB];
    sa[b] = k;
    b++;

    for (int32_t i = 0; i < textLen; i++) {
        int32_t j = sa[i];
        if (j == 0) continue;
        if (j < 0) {
            sa[i] = -j;
            continue;
        }
        sa[i] = 0;
        k = j - 1;
        c0 = text[k - 1];
        c1 = text[k];
        if (c0 < c1) k = -k;

        if (cB != c1) {
            bucket[cB] = b;
            cB = c1;
            b = bucket[cB];
        }
        sa[b] = k;
        b++;
    }
}

static inline void induceSubS_32(const int32_t* text, int32_t textLen, int32_t textMax,
                                 int32_t* sa, int32_t* freq, int32_t* bucket)
{
    bucketMax_32(text, textLen, textMax, freq, bucket);
    int32_t cB = 0;
    int32_t b = bucket[cB];
    int32_t top = textLen;

    for (int32_t i = textLen - 1; i >= 0; i--) {
        int32_t j = sa[i];
        if (j == 0) continue;
        sa[i] = 0;
        if (j < 0) {
            top--;
            sa[top] = -j;
            continue;
        }
        int32_t k = j - 1;
        int32_t c1_val = text[k];
        int32_t c0_val = text[k - 1];
        if (c0_val > c1_val) k = -k;

        if (cB != c1_val) {
            bucket[cB] = b;
            cB = c1_val;
            b = bucket[cB];
        }
        b--;
        sa[b] = k;
    }
}

static inline void length_32(const int32_t* text, int32_t textLen,
                            int32_t* sa, int32_t numLMS)
{
    int32_t end = 0;
    int32_t c0 = 0, c1 = 0;
    bool isTypeS = false;
    for (int32_t i = textLen - 1; i >= 0; i--) {
        c1 = c0;
        c0 = text[i];
        if (c0 < c1) {
            isTypeS = true;
        } else if (c0 > c1 && isTypeS) {
            isTypeS = false;
            int32_t j = i + 1;
            int32_t code = (end == 0) ? 0 : (end - j);
            sa[j >> 1] = code;
            end = j + 1;
        }
    }
}

static inline int32_t assignID_32(const int32_t* text, int32_t textLen,
                                 int32_t* sa, int32_t numLMS)
{
    int32_t id = 0;
    int32_t lastLen = -1;
    int32_t lastPos = 0;
    int32_t* lmsPtr = sa + textLen - numLMS;

    for (int32_t idx = 0; idx < numLMS; idx++) {
        int32_t j = lmsPtr[idx];
        int32_t nVal = sa[j >> 1];
        bool isNew = true;

        if (nVal == lastLen) {
            if (static_cast<uint32_t>(nVal) >= static_cast<uint32_t>(textLen)) {
                isNew = false;
            } else {
                int32_t len = nVal;
                bool same = true;
                for (int32_t i = 0; i < len; i++) {
                    if (text[j + i] != text[lastPos + i]) {
                        same = false;
                        break;
                    }
                }
                if (same) isNew = false;
            }
        }

        if (isNew) {
            id++;
            lastPos = j;
            lastLen = nVal;
        }
        sa[j >> 1] = id;
    }
    return id;
}

static inline void unmap_32(const int32_t* text, int32_t textLen,
                            int32_t* sa, int32_t numLMS)
{
    int32_t* unmap = sa + textLen - numLMS;
    int32_t j = numLMS;

    int32_t c0 = 0, c1 = 0;
    bool isTypeS = false;
    for (int32_t i = textLen - 1; i >= 0; i--) {
        c1 = c0;
        c0 = text[i];
        if (c0 < c1) {
            isTypeS = true;
        } else if (c0 > c1 && isTypeS) {
            isTypeS = false;
            j--;
            unmap[j] = i + 1;
        }
    }

    for (int32_t i = 0; i < numLMS; i++) {
        sa[i] = unmap[sa[i]];
    }
}

static inline void expand_32(const int32_t* text, int32_t textLen, int32_t textMax,
                             int32_t* freq, int32_t* bucket, int32_t* sa, int32_t numLMS)
{
    bucketMax_32(text, textLen, textMax, freq, bucket);
    int32_t x = numLMS - 1;
    int32_t saX = sa[x];
    int32_t c = text[saX];
    int32_t b = bucket[c] - 1;
    bucket[c] = b;

    for (int32_t i = textLen - 1; i >= 0; i--) {
        if (i != b) {
            sa[i] = 0;
            continue;
        }
        sa[i] = saX;
        if (x > 0) {
            x--;
            saX = sa[x];
            c = text[saX];
            b = bucket[c] - 1;
            bucket[c] = b;
        }
    }
}

static inline void induceL_32(const int32_t* text, int32_t textLen, int32_t textMax,
                             int32_t* sa, int32_t* freq, int32_t* bucket)
{
    bucketMin_32(text, textLen, textMax, freq, bucket);
    int32_t k = textLen - 1;
    int32_t c0 = text[k - 1];
    int32_t c1 = text[k];
    if (c0 < c1) k = -k;

    int32_t cB = c1;
    int32_t b = bucket[cB];
    sa[b] = k;
    b++;

    for (int32_t i = 0; i < textLen; i++) {
        int32_t j = sa[i];
        if (j <= 0) continue;

        k = j - 1;
        c1 = text[k];
        if (k > 0) {
            c0 = text[k - 1];
            if (c0 < c1) k = -k;
        }

        if (cB != c1) {
            bucket[cB] = b;
            cB = c1;
            b = bucket[cB];
        }
        sa[b] = k;
        b++;
    }
}

static inline void induceS_32(const int32_t* text, int32_t textLen, int32_t textMax,
                              int32_t* sa, int32_t* freq, int32_t* bucket)
{
    bucketMax_32(text, textLen, textMax, freq, bucket);
    int32_t cB = 0;
    int32_t b = bucket[cB];

    for (int32_t i = textLen - 1; i >= 0; i--) {
        int32_t j = sa[i];
        if (j >= 0) continue;

        j = -j;
        sa[i] = j;

        int32_t k = j - 1;
        int32_t c1 = text[k];
        if (k > 0) {
            int32_t c0 = text[k - 1];
            if (c0 <= c1) k = -k;
        }

        if (cB != c1) {
            bucket[cB] = b;
            cB = c1;
            b = bucket[cB];
        }
        b--;
        sa[b] = k;
    }
}

//------------------------------------------------------------------------------
// Forward declaration (recurse_32 calls sais_32, sais_32 calls recurse_32)
//------------------------------------------------------------------------------
static inline void recurse_32(int32_t* sa, int32_t* oldTmp, int32_t n,
                              int32_t numLMS, int32_t maxID, int32_t oldTmpLen);

//------------------------------------------------------------------------------
// sais_32 - recursive variant for int32 text
//------------------------------------------------------------------------------
static inline void sais_32(const int32_t* text, int32_t textLen, int32_t textMax,
                          int32_t* sa, int32_t* tmp, int32_t tmpLen)
{
    if (textLen == 0) return;
    if (textLen == 1) {
        sa[0] = 0;
        return;
    }

    int32_t* freq = nullptr;
    int32_t* bucket = tmp;
    if (tmpLen >= 2 * textMax) {
        freq = tmp;
        bucket = tmp + textMax;
        freq[0] = -1;
    }

    int32_t numLMS = placeLMS_32(text, textLen, sa, freq, bucket, textMax);
    if (numLMS <= 1) {
        // 0 or 1 items already sorted
    } else {
        induceSubL_32(text, textLen, textMax, sa, freq, bucket);
        induceSubS_32(text, textLen, textMax, sa, freq, bucket);
        length_32(text, textLen, sa, numLMS);
        int32_t maxID = assignID_32(text, textLen, sa, numLMS);
        if (maxID < numLMS) {
            map_32(sa, textLen, numLMS);
            recurse_32(sa, tmp, textLen, numLMS, maxID, tmpLen);
            unmap_32(text, textLen, sa, numLMS);
        } else {
            std::memcpy(sa, sa + textLen - numLMS, static_cast<size_t>(numLMS) * sizeof(int32_t));
        }
        expand_32(text, textLen, textMax, freq, bucket, sa, numLMS);
    }
    induceL_32(text, textLen, textMax, sa, freq, bucket);
    induceS_32(text, textLen, textMax, sa, freq, bucket);

    tmp[0] = -1;
}

//------------------------------------------------------------------------------
// recurse_32 - sets up tmp and calls sais_32
//------------------------------------------------------------------------------
static inline void recurse_32(int32_t* sa, int32_t* oldTmp, int32_t n,
                              int32_t numLMS, int32_t maxID, int32_t oldTmpLen)
{
    int32_t* dst = sa;
    int32_t* saTmp = sa + numLMS;
    int32_t* text = sa + n - numLMS;
    int32_t saTmpLen = n - 2 * numLMS;

    int32_t* tmp = oldTmp;
    int32_t tmpLen = oldTmpLen;
    if (oldTmpLen < saTmpLen) {
        tmp = saTmp;
        tmpLen = saTmpLen;
    }

    std::vector<int32_t> allocTmp;
    if (tmpLen < numLMS) {
        int32_t allocSize = maxID;
        if (allocSize < numLMS / 2) allocSize = numLMS / 2;
        allocTmp.resize(static_cast<size_t>(allocSize));
        tmp = allocTmp.data();
        tmpLen = allocSize;
    }

    for (int32_t i = 0; i < numLMS; i++) dst[i] = 0;
    sais_32(text, numLMS, maxID, dst, tmp, tmpLen);
}

//------------------------------------------------------------------------------
// sais_8_32 - main algorithm for byte text
//------------------------------------------------------------------------------
static inline void sais_8_32(const uint8_t* text, int32_t n, int32_t textMax,
                            int32_t* sa, int32_t* tmp, int32_t tmpLen)
{
    if (n == 0) return;
    if (n == 1) {
        sa[0] = 0;
        return;
    }

    int32_t* freq = nullptr;
    int32_t* bucket = tmp;
    if (tmpLen >= 2 * textMax) {
        freq = tmp;
        bucket = tmp + textMax;
        freq[0] = -1;
    }

    int32_t numLMS = placeLMS_8_32(text, n, sa, freq, bucket);
    if (numLMS <= 1) {
        // 0 or 1 items already sorted
    } else {
        induceSubL_8_32(text, n, sa, freq, bucket);
        induceSubS_8_32(text, n, sa, freq, bucket);
        length_8_32(text, n, sa, numLMS);
        int32_t maxID = assignID_8_32(text, n, sa, numLMS);
        if (maxID < numLMS) {
            map_32(sa, n, numLMS);
            recurse_32(sa, tmp, n, numLMS, maxID, tmpLen);
            unmap_8_32(text, n, sa, numLMS);
        } else {
            std::memcpy(sa, sa + n - numLMS, static_cast<size_t>(numLMS) * sizeof(int32_t));
        }
        expand_8_32(text, n, freq, bucket, sa, numLMS);
    }
    induceL_8_32(text, n, sa, freq, bucket);
    induceS_8_32(text, n, sa, freq, bucket);

    tmp[0] = -1;
}

//------------------------------------------------------------------------------
// build_suffix_array - public entry point
//------------------------------------------------------------------------------
static inline void build_suffix_array(const uint8_t* text, int32_t* sa, int32_t n, int32_t* tmp)
{
    if (text == nullptr || sa == nullptr || n < 0) return;
    if (static_cast<uint32_t>(n) > 0x7FFFFFFFu) return;  // overflow check

    std::memset(sa, 0, static_cast<size_t>(n) * sizeof(int32_t));
    sais_8_32(text, n, 256, sa, tmp, 512);
}


static inline void build_suffix_array(const uint8_t* text, int32_t* sa, int32_t n)
{
    int32_t tmp[512];
    build_suffix_array(text, sa, n, tmp);
}

}  // namespace astrobwt
}  // namespace xmrig

#endif  // XMRIG_ASTROBWT_SAIS_H
