/* XMRig
 * Copyright (c) 2018-2024 SChernykh   <https://github.com/SChernykh>
 * Copyright (c) 2016-2024 XMRig       <https://github.com/xmrig>, <support@xmrig.com>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "crypto/astrobwt/AstroBWT.h"
#include "crypto/astrobwt/sha256_utils.h"
#include "crypto/astrobwt/hash_utils.h"
#include "crypto/astrobwt/divsufsort.h"
#include "crypto/astrobwt/sais.h"
#include "crypto/astrobwt/Salsa20.hpp"
#include "crypto/astrobwt/wolf/wolf.h"
#include "crypto/astrobwt/astrobwt_suffix.h"
#include "crypto/astrobwt/spsa/spsa_finalize.h"
#include "crypto/cn/CryptoNight.h"

#include <cstring>
#include <mutex>


namespace xmrig {
namespace astrobwt {

static std::once_flag s_init_once;
static bool s_initialized = false;


void init()
{
    std::call_once(s_init_once, []() {
        sha256_init_hw();
#       if defined(XMRIG_WOLF_ENABLED)
        wolf::init();
#       endif
        spsa::init();
        s_initialized = true;
    });
}


bool astrobwt_dero_v3(const void* input_data, uint32_t input_size, void* scratchpad, uint8_t* output_hash)
{
    auto* scratch = reinterpret_cast<ScratchData*>(static_cast<uint8_t*>(scratchpad) + 64);

#   if defined(__aarch64__) && defined(XMRIG_ASTRO_AARCH64_ENABLED)
    if (spsa::available()) {
        if (astrobwt_dero_v3_aarch64(input_data, input_size, scratch, output_hash)) {
            return true;
        }
    }
#   endif

#   if defined(XMRIG_WOLF_ENABLED)
    if (wolf::available()) {
        return wolf::astrobwt_dero_v3_wolf(input_data, input_size, scratch, output_hash);
    }
#   endif

    uint8_t step_3[256];
    memset(step_3, 0, sizeof(step_3));

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

    for (;;) {
        tries++;

        uint64_t random_switcher = prev_lhash ^ lhash ^ tries;

        uint8_t op   = static_cast<uint8_t>(random_switcher);
        uint8_t pos1 = static_cast<uint8_t>(random_switcher >> 8);
        uint8_t pos2 = static_cast<uint8_t>(random_switcher >> 16);

        if (pos1 > pos2) {
            uint8_t tmp = pos1;
            pos1 = pos2;
            pos2 = tmp;
        }

        if (static_cast<uint8_t>(pos2 - pos1) > 32) {
            pos2 = pos1 + ((pos2 - pos1) & 0x1f);
        }

#include "crypto/astrobwt/astrobwt_ops.inl"

        if (static_cast<uint8_t>(s[pos1] - s[pos2]) < 0x10) {
            prev_lhash = lhash + prev_lhash;
            lhash = xxhash64(s, pos2);
        }

        if (static_cast<uint8_t>(s[pos1] - s[pos2]) < 0x20) {
            prev_lhash = lhash + prev_lhash;
            lhash = fnv1a_hash64(s, pos2);
        }

        if (static_cast<uint8_t>(s[pos1] - s[pos2]) < 0x30) {
            prev_lhash = lhash + prev_lhash;
            lhash = siphash(tries, prev_lhash, s, pos2);
        }

        if (static_cast<uint8_t>(s[pos1] - s[pos2]) <= 0x40) {
            rc4.xor_stream(s, s, 256);
        }

        s[255] = s[255] ^ s[pos1] ^ s[pos2];

        copy256(scratch->data + (tries - 1) * 256, s);

        if (tries > 260 + 16 || (s[255] >= 0xf0 && tries > 260)) {
            break;
        }
    }

    uint32_t data_len = static_cast<uint32_t>(
        ((tries - 4) * 256) + (((static_cast<uint64_t>(s[253]) << 8) | static_cast<uint64_t>(s[254])) & 0x3ff)
    );

    if (data_len > MAX_LENGTH) {
        data_len = MAX_LENGTH;
    }

    suffix_hash(scratch->data, static_cast<int32_t>(data_len), scratch, output_hash);

    return true;
}


template<>
void single_hash<Algorithm::ASTROBWT_DERO_3>(const uint8_t* input, size_t size, uint8_t* output, cryptonight_ctx** ctx, uint64_t)
{
    init();

    astrobwt_dero_v3(input, static_cast<uint32_t>(size), ctx[0]->memory, output);
}

} // namespace astrobwt
} // namespace xmrig
