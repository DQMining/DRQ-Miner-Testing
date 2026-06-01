#include "crypto/astrobwt/wolf/wolf.h"

#include "crypto/astrobwt/AstroBWT.h"
#include "crypto/astrobwt/Salsa20.hpp"
#include "crypto/astrobwt/astrobwt_suffix.h"
#include "crypto/astrobwt/hash_utils.h"
#include "crypto/astrobwt/sha256_utils.h"
#include "crypto/astrobwt/spsa/spsa_finalize.h"
#include "crypto/astrobwt/wolf/wolf_simd.h"
#include "crypto/astrobwt/wolf/wolf_tables.h"
#include "crypto/astrobwt/wolf/wolf_worker.h"

#include <algorithm>
#include <cstring>

#if defined(__AVX2__) || (defined(_MSC_VER) && defined(__AVX2__))

namespace xmrig {
namespace astrobwt {
namespace wolf {

namespace {

inline void wolf_permute_inplace(uint8_t* s, uint16_t op, uint8_t pos1, uint8_t pos2)
{
    const uint32_t opcode = CodeLUT_16[op];
    __m256i data = _mm256_loadu_si256(reinterpret_cast<__m256i*>(&s[pos1]));
    const __m256i old = data;

    WOLF_BRANCH_SIMD(data, s[pos2], opcode);

    data = _mm256_blendv_epi8(old, data, genMask_avx2(pos2 - pos1));
    _mm256_storeu_si256(reinterpret_cast<__m256i*>(&s[pos1]), data);
}

inline uint8_t normA(uint8_t pos1, uint8_t pos2, const uint8_t* s)
{
    int a = static_cast<int>(s[pos1]) - static_cast<int>(s[pos2]);
    return static_cast<uint8_t>((256 + (a % 256)) % 256);
}

} // namespace

bool available()
{
    return true;
}

void init()
{
    init_wolf_lut();
}

bool astrobwt_dero_v3_wolf(const void* input_data, uint32_t input_size, ScratchData* scratch, uint8_t* output_hash)
{
    WolfWorker& worker = thread_worker();
    uint8_t* const blob = worker.sData ? worker.sData : scratch->data;

    worker.templateIdx = 0;

    uint8_t step_3[256];
    std::memset(step_3, 0, sizeof(step_3));

    uint8_t sha_key[32];
    sha256_auto(static_cast<const uint8_t*>(input_data), input_size, sha_key);

    const uint64_t iv = 0;
    ZeroTier::Salsa20 salsa(sha_key, &iv);
    salsa.XORKeyStream(step_3, 256);

    RC4 rc4;
    rc4.init(step_3, 256);
    rc4.xor_stream(step_3, step_3, 256);

    uint64_t lhash = fnv1a_hash64(step_3, 256);
    uint64_t prev_lhash = lhash;

    uint64_t tries = 0;
    uint8_t* s = step_3;
    uint8_t chunkCount = 1;
    int firstChunk = 0;
    uint8_t lp1 = 0;
    uint8_t lp2 = 255;

    for (;;) {
        tries++;

        const uint64_t random_switcher = prev_lhash ^ lhash ^ tries;

        uint8_t pos1 = static_cast<uint8_t>(random_switcher >> 8);
        uint8_t pos2 = static_cast<uint8_t>(random_switcher >> 16);

        if (pos1 > pos2) {
            const uint8_t tmp = pos1;
            pos1 = pos2;
            pos2 = tmp;
        }

        if (static_cast<uint8_t>(pos2 - pos1) > 32) {
            pos2 = static_cast<uint8_t>(pos1 + ((pos2 - pos1) & 0x1f));
        }

        if (tries > 1) {
            lp1 = std::min(lp1, pos1);
            lp2 = std::max(lp2, pos2);
        }

        const uint8_t op = static_cast<uint8_t>(random_switcher);

        if (op == 253) {
            for (uint8_t i = pos1; i < pos2; ++i) {
                s[i] = rotl8(s[i], 3);
                s[i] ^= rotl8(s[i], 2);
                s[i] ^= s[pos2];
                s[i] = rotl8(s[i], 3);
                prev_lhash = lhash + prev_lhash;
                lhash = xxhash64(s, pos2);
            }
        }
        else if (op >= 254) {
            rc4.init(s, 256);
            for (uint8_t i = pos1; i < pos2; ++i) {
                s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
                s[i] = rotl8(s[i], 3);
                s[i] ^= rotl8(s[i], 2);
                s[i] = rotl8(s[i], 3);
            }
        }
        else {
            wolf_permute_inplace(s, op, pos1, pos2);

            if (!op && ((pos2 - pos1) % 2 == 1)) {
                const uint8_t t1 = s[pos1];
                const uint8_t t2 = s[pos2];
                s[pos1] = reverse8(t2);
                s[pos2] = reverse8(t1);
            }
        }

        uint8_t pushPos1 = lp1;
        uint8_t pushPos2 = lp2;
        if (pos1 == pos2) {
            pushPos1 = static_cast<uint8_t>(-1);
            pushPos2 = static_cast<uint8_t>(-1);
        }

        const uint8_t A = normA(pos1, pos2, s);

        if (A < 0x10) {
            prev_lhash = lhash + prev_lhash;
            lhash = xxhash64(s, pos2);
        }

        if (A < 0x20) {
            prev_lhash = lhash + prev_lhash;
            lhash = fnv1a_hash64(s, pos2);
        }

        if (A < 0x30) {
            prev_lhash = lhash + prev_lhash;
            lhash = siphash(tries, prev_lhash, s, pos2);
        }

        if (A <= 0x40) {
            rc4.xor_stream(s, s, 256);

            if (255 - pushPos2 < wolf::kMinPrefLen) {
                pushPos2 = 255;
            }
            if (pushPos1 < wolf::kMinPrefLen) {
                pushPos1 = 0;
            }
            if (pushPos1 == 255) {
                pushPos1 = 0;
            }

            worker.astroTemplate[worker.templateIdx] = wolf::TemplateMarker{
                static_cast<uint8_t>(chunkCount > 1 ? pushPos1 : 0),
                static_cast<uint8_t>(chunkCount > 1 ? pushPos2 : 255),
                0,
                0,
                static_cast<uint16_t>((firstChunk << 7) | chunkCount)
            };

            pushPos1 = 0;
            pushPos2 = 255;
            worker.templateIdx += (tries > 1) ? 1 : 0;
            firstChunk = static_cast<int>(tries - 1);
            lp1 = 255;
            lp2 = 0;
            chunkCount = 1;
        }
        else {
            chunkCount++;
        }

        s[255] = static_cast<uint8_t>(s[255] ^ s[pos1] ^ s[pos2]);

        if (255 - pushPos2 < wolf::kMinPrefLen) {
            pushPos2 = 255;
        }
        if (pushPos1 < wolf::kMinPrefLen) {
            pushPos1 = 0;
        }

        copy256(blob + (tries - 1) * 256, s);

        if (tries > 260 + 16 || (s[255] >= 0xf0 && tries > 260)) {
            break;
        }
    }

    if (chunkCount > 0) {
        if (255 - lp2 < wolf::kMinPrefLen) {
            lp2 = 255;
        }
        if (lp1 < wolf::kMinPrefLen) {
            lp1 = 0;
        }
        worker.astroTemplate[worker.templateIdx] = wolf::TemplateMarker{
            static_cast<uint8_t>(chunkCount > 1 ? lp1 : 0),
            static_cast<uint8_t>(chunkCount > 1 ? lp2 : 255),
            0,
            0,
            static_cast<uint16_t>((firstChunk << 7) | chunkCount)
        };
        worker.templateIdx++;
    }

    uint32_t data_len = static_cast<uint32_t>(
        ((tries - 4) * 256) + (((static_cast<uint64_t>(s[253]) << 8) | static_cast<uint64_t>(s[254])) & 0x3ff)
    );

    if (data_len > MAX_LENGTH) {
        data_len = MAX_LENGTH;
    }

    static_assert(sizeof(wolf::TemplateMarker) == sizeof(spsa::SpsaTemplateMarker),
                  "SPSA template layout must match wolf TemplateMarker");

    suffix_hash(blob,
                static_cast<int32_t>(data_len),
                scratch,
                output_hash,
                reinterpret_cast<const spsa::SpsaTemplateMarker*>(worker.astroTemplate),
                worker.templateIdx);

    return true;
}

} // namespace wolf
} // namespace astrobwt
} // namespace xmrig

#endif // __AVX2__
