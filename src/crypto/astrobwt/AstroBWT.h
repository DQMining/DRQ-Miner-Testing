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

#ifndef XMRIG_ASTROBWT_H
#define XMRIG_ASTROBWT_H

#include <cstdint>
#include <cstddef>

#include "base/crypto/Algorithm.h"

struct cryptonight_ctx;

namespace xmrig {
namespace astrobwt {

constexpr uint32_t MAX_LENGTH = (256 * 384) - 1;
constexpr uint32_t DERO_MINIBLOCK_SIZE = 48;

struct ScratchData {
    uint8_t  data[MAX_LENGTH + 64];
    int32_t  sa[MAX_LENGTH];
    int32_t  sais_tmp[512];
    int32_t  bucket_a[256];
    int32_t  bucket_b[256 * 256];
    uint8_t* sa_bytes() { return reinterpret_cast<uint8_t*>(sa); }
};

bool astrobwt_dero_v3(const void* input_data, uint32_t input_size, void* scratchpad, uint8_t* output_hash);

#if defined(__aarch64__) && defined(XMRIG_ASTRO_AARCH64_ENABLED)
bool astrobwt_dero_v3_aarch64(const void* input_data, uint32_t input_size, ScratchData* scratch, uint8_t* output_hash);
#endif

void init();

template<Algorithm::Id ALGO>
void single_hash(const uint8_t* input, size_t size, uint8_t* output, cryptonight_ctx** ctx, uint64_t);

template<>
void single_hash<Algorithm::ASTROBWT_DERO_3>(const uint8_t* input, size_t size, uint8_t* output, cryptonight_ctx** ctx, uint64_t);

} // namespace astrobwt
} // namespace xmrig

#endif
