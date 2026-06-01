#ifndef XMRIG_ASTROBWT_WOLF_SIMD_H
#define XMRIG_ASTROBWT_WOLF_SIMD_H

#if defined(__AVX2__) || (defined(_MSC_VER) && defined(__AVX2__))

#   include <immintrin.h>

namespace xmrig {
namespace astrobwt {
namespace wolf {

inline __m256i genMask_avx2(int bytes)
{
    const __m256i sequence = _mm256_setr_epi8(
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31);

    bytes = (bytes < 0) ? 0 : (bytes > 32) ? 32 : bytes;

    const __m256i count = _mm256_set1_epi8(bytes);
    return _mm256_cmpgt_epi8(count, sequence);
}

inline __m256i mullo_epi8_256(__m256i x, __m256i y)
{
    const __m256i mask1 = _mm256_set1_epi16(0xFF00);
    const __m256i mask2 = _mm256_set1_epi16(0x00FF);

    __m256i aa = _mm256_srli_epi16(_mm256_and_si256(x, mask1), 8);
    __m256i ab = _mm256_srli_epi16(_mm256_and_si256(y, mask1), 8);
    __m256i ba = _mm256_and_si256(x, mask2);
    __m256i bb = _mm256_and_si256(y, mask2);

    __m256i pa = _mm256_slli_epi16(_mm256_mullo_epi16(aa, ab), 8);
    __m256i pb = _mm256_mullo_epi16(ba, bb);

    pa = _mm256_and_si256(pa, mask1);
    pb = _mm256_and_si256(pb, mask2);

    return _mm256_or_si256(pa, pb);
}

inline __m256i sllv_epi8_256(__m256i a, __m256i count)
{
    const __m256i mask_hi        = _mm256_set1_epi32(0xFF00FF00);
    const __m256i multiplier_lut = _mm256_set_epi8(
        0, 0, 0, 0, 0, 0, 0, 0, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01,
        0, 0, 0, 0, 0, 0, 0, 0, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01);

    __m256i count_sat  = _mm256_min_epu8(count, _mm256_set1_epi8(8));
    __m256i multiplier = _mm256_shuffle_epi8(multiplier_lut, count_sat);
    __m256i x_lo       = _mm256_mullo_epi16(a, multiplier);

    __m256i multiplier_hi = _mm256_srli_epi16(multiplier, 8);
    __m256i a_hi           = _mm256_and_si256(a, mask_hi);
    __m256i x_hi          = _mm256_mullo_epi16(a_hi, multiplier_hi);

    return _mm256_blendv_epi8(x_lo, x_hi, mask_hi);
}

inline __m256i srlv_epi8_256(__m256i a, __m256i count)
{
    const __m256i mask_hi        = _mm256_set1_epi32(0xFF00FF00);
    const __m256i multiplier_lut = _mm256_set_epi8(
        0, 0, 0, 0, 0, 0, 0, 0, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
        0, 0, 0, 0, 0, 0, 0, 0, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80);

    __m256i count_sat      = _mm256_min_epu8(count, _mm256_set1_epi8(8));
    __m256i multiplier     = _mm256_shuffle_epi8(multiplier_lut, count_sat);
    __m256i a_lo           = _mm256_andnot_si256(mask_hi, a);
    __m256i multiplier_lo  = _mm256_andnot_si256(mask_hi, multiplier);
    __m256i x_lo           = _mm256_mullo_epi16(a_lo, multiplier_lo);
    x_lo                   = _mm256_srli_epi16(x_lo, 7);

    __m256i multiplier_hi = _mm256_and_si256(mask_hi, multiplier);
    __m256i x_hi          = _mm256_mulhi_epu16(a, multiplier_hi);
    x_hi                  = _mm256_slli_epi16(x_hi, 1);

    return _mm256_blendv_epi8(x_lo, x_hi, mask_hi);
}

inline __m256i rolv_epi8_256(__m256i x, __m256i y)
{
    __m256i y_mod      = _mm256_and_si256(y, _mm256_set1_epi8(7));
    __m256i left_shift = sllv_epi8_256(x, y_mod);
    __m256i right_shift_counts = _mm256_sub_epi8(_mm256_set1_epi8(8), y_mod);
    __m256i right_shift = srlv_epi8_256(x, right_shift_counts);
    return _mm256_or_si256(left_shift, right_shift);
}

inline __m256i rol_epi8_256(__m256i x, int r)
{
    __m256i mask1 = _mm256_set1_epi16(0x00FF);
    __m256i mask2 = _mm256_set1_epi16(0xFF00);
    __m256i a     = _mm256_and_si256(x, mask1);
    __m256i b     = _mm256_and_si256(x, mask2);

    __m256i rotatedA = _mm256_or_si256(_mm256_slli_epi16(a, r), _mm256_srli_epi16(a, 8 - r));
    rotatedA         = _mm256_and_si256(rotatedA, mask1);

    __m256i rotatedB = _mm256_or_si256(_mm256_slli_epi16(b, r), _mm256_srli_epi16(b, 8 - r));
    rotatedB         = _mm256_and_si256(rotatedB, mask2);

    return _mm256_or_si256(rotatedA, rotatedB);
}

inline __m128i parallelPopcnt16bytes(__m128i xmm)
{
    const __m128i mask4  = _mm_set1_epi8(0x0F);
    const __m128i lookup = _mm_setr_epi8(0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4);
    __m128i low  = _mm_and_si128(mask4, xmm);
    __m128i high = _mm_and_si128(mask4, _mm_srli_epi16(xmm, 4));
    return _mm_add_epi8(_mm_shuffle_epi8(lookup, low), _mm_shuffle_epi8(lookup, high));
}

inline __m256i popcnt256_epi8(__m256i data)
{
    __m128i hi = _mm256_extractf128_si256(data, 1);
    __m128i lo = _mm256_castsi256_si128(data);
    hi = parallelPopcnt16bytes(hi);
    lo = parallelPopcnt16bytes(lo);
    return _mm256_set_m128i(hi, lo);
}

inline __m256i reverse_epi8_256(__m256i input)
{
    const __m256i mask_0f = _mm256_set1_epi8(0x0F);
    const __m256i mask_33 = _mm256_set1_epi8(0x33);
    const __m256i mask_55 = _mm256_set1_epi8(0x55);
    const __m256i all_ff  = _mm256_set1_epi8(static_cast<char>(0xFF));

    __m256i temp = _mm256_and_si256(input, mask_0f);
    temp         = _mm256_slli_epi16(temp, 4);
    input        = _mm256_and_si256(input, _mm256_andnot_si256(mask_0f, all_ff));
    input        = _mm256_srli_epi16(input, 4);
    input        = _mm256_or_si256(input, temp);

    temp  = _mm256_and_si256(input, mask_33);
    temp  = _mm256_slli_epi16(temp, 2);
    input = _mm256_and_si256(input, _mm256_andnot_si256(mask_33, all_ff));
    input = _mm256_srli_epi16(input, 2);
    input = _mm256_or_si256(input, temp);

    temp  = _mm256_and_si256(input, mask_55);
    temp  = _mm256_slli_epi16(temp, 1);
    input = _mm256_and_si256(input, _mm256_andnot_si256(mask_55, all_ff));
    input = _mm256_srli_epi16(input, 1);
    input = _mm256_or_si256(input, temp);

    return input;
}

#   define WOLF_POPCNT256_EPI8(x) popcnt256_epi8(x)

#   define WOLF_BRANCH_SIMD(IN, POS2VAL, OPCODE)                                   \
    {                                                                              \
        const __m256i _wb_vec3 = _mm256_set1_epi8(3);                              \
        for (int _wb_i = 3; _wb_i >= 0; --_wb_i) {                                 \
            const uint8_t _wb_insn = static_cast<uint8_t>(((OPCODE) >> (_wb_i << 2)) & 0xF); \
            switch (_wb_insn) {                                                    \
            case 0:  (IN) = _mm256_add_epi8((IN), (IN)); break;                    \
            case 1:  (IN) = _mm256_sub_epi8((IN),                                   \
                         _mm256_xor_si256((IN), _mm256_set1_epi8(97))); break;     \
            case 2:  (IN) = mullo_epi8_256((IN), (IN)); break;                      \
            case 3:  (IN) = _mm256_xor_si256((IN), _mm256_set1_epi8((POS2VAL))); break; \
            case 4:  (IN) = _mm256_xor_si256((IN), _mm256_set1_epi64x(-1LL)); break; \
            case 5:  (IN) = _mm256_and_si256((IN), _mm256_set1_epi8((POS2VAL))); break; \
            case 6:  (IN) = sllv_epi8_256((IN), _mm256_and_si256((IN), _wb_vec3)); break; \
            case 7:  (IN) = srlv_epi8_256((IN), _mm256_and_si256((IN), _wb_vec3)); break; \
            case 8:  (IN) = reverse_epi8_256((IN)); break;                        \
            case 9:  (IN) = _mm256_xor_si256((IN), WOLF_POPCNT256_EPI8((IN))); break; \
            case 10: (IN) = rolv_epi8_256((IN), (IN)); break;                     \
            case 11: (IN) = rol_epi8_256((IN), 1); break;                          \
            case 12: (IN) = _mm256_xor_si256((IN), rol_epi8_256((IN), 2)); break;  \
            case 13: (IN) = rol_epi8_256((IN), 3); break;                          \
            case 14: (IN) = _mm256_xor_si256((IN), rol_epi8_256((IN), 4)); break;  \
            case 15: (IN) = rol_epi8_256((IN), 5); break;                         \
            }                                                                      \
        }                                                                          \
    }

} // namespace wolf
} // namespace astrobwt
} // namespace xmrig

#endif // AVX2

#endif
