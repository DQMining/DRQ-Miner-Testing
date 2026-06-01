#ifndef XMRIG_ASTROBWT_SA_OPT_H
#define XMRIG_ASTROBWT_SA_OPT_H

#include <cstdint>
#include <cstring>

namespace xmrig {
namespace astrobwt {

constexpr uint32_t SA_OPT_MAX = (256 * 384);

static inline void build_suffix_array_opt(const uint8_t* text, int32_t* sa, int32_t n)
{
    if (n <= 0) return;
    if (n == 1) { sa[0] = 0; return; }

    static thread_local int32_t rank_a[SA_OPT_MAX];
    static thread_local int32_t rank_b[SA_OPT_MAX];
    static thread_local int32_t tmp_sa[SA_OPT_MAX];
    static thread_local int32_t cnt[SA_OPT_MAX + 256];

    int32_t* rk = rank_a;
    int32_t* nrk = rank_b;

    // Phase 1: sort by first character
    memset(cnt, 0, 256 * sizeof(int32_t));
    for (int32_t i = 0; i < n; i++) cnt[text[i]]++;
    for (int32_t i = 1; i < 256; i++) cnt[i] += cnt[i - 1];
    for (int32_t i = n - 1; i >= 0; i--) sa[--cnt[text[i]]] = i;

    rk[sa[0]] = 0;
    int32_t classes = 1;
    for (int32_t i = 1; i < n; i++) {
        if (text[sa[i]] != text[sa[i - 1]]) classes++;
        rk[sa[i]] = classes - 1;
    }

    for (int32_t gap = 1; classes < n; gap <<= 1) {
        // Stable sort by second key (rk[i + gap], treating out-of-bounds as -1)
        // Use counting sort with range [0..classes-1] for valid, plus one slot for "no second key"
        // Encode: second_key = (i + gap < n) ? rk[i + gap] + 1 : 0
        int32_t range = classes + 1;

        memset(cnt, 0, (range + 1) * sizeof(int32_t));
        for (int32_t i = 0; i < n; i++) {
            int32_t sk = (sa[i] + gap < n) ? rk[sa[i] + gap] + 1 : 0;
            cnt[sk]++;
        }
        for (int32_t i = 1; i <= range; i++) cnt[i] += cnt[i - 1];
        for (int32_t i = n - 1; i >= 0; i--) {
            int32_t sk = (sa[i] + gap < n) ? rk[sa[i] + gap] + 1 : 0;
            tmp_sa[--cnt[sk]] = sa[i];
        }

        // Stable sort by first key (rk[i])
        memset(cnt, 0, (classes + 1) * sizeof(int32_t));
        for (int32_t i = 0; i < n; i++) cnt[rk[tmp_sa[i]]]++;
        for (int32_t i = 1; i < classes; i++) cnt[i] += cnt[i - 1];
        for (int32_t i = n - 1; i >= 0; i--) {
            sa[--cnt[rk[tmp_sa[i]]]] = tmp_sa[i];
        }

        // Recompute ranks
        nrk[sa[0]] = 0;
        classes = 1;
        for (int32_t i = 1; i < n; i++) {
            int32_t p = sa[i - 1], c = sa[i];
            bool same = (rk[p] == rk[c]);
            if (same) {
                int32_t sp = (p + gap < n) ? rk[p + gap] : -1;
                int32_t sc = (c + gap < n) ? rk[c + gap] : -1;
                same = (sp == sc);
            }
            if (!same) classes++;
            nrk[c] = classes - 1;
        }

        int32_t* t = rk; rk = nrk; nrk = t;
        if (classes == n) break;
    }
}

} // namespace astrobwt
} // namespace xmrig

#endif
