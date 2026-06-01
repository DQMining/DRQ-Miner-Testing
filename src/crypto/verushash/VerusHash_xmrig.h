/* XMRig VerusHash integration wrapper
 *
 * Thin glue between VerusHash reference implementation and xmrig's
 * cn_hash_fun interface (single-hash path only).
 */

#ifndef XMRIG_VERUSHASH_XMRIG_H
#define XMRIG_VERUSHASH_XMRIG_H

#include <cstddef>
#include <cstdint>

#include "base/crypto/Algorithm.h"
#include "crypto/verushash/verus_hash.h"


struct cryptonight_ctx;


namespace xmrig {
namespace verushash {

void init();
void prepare_fast_job(const uint8_t *input, size_t size, uint64_t cacheToken);
bool prepared_hot_is_rolling_nonce32();
bool prepared_hot_is_dynamic_nonce4();
bool prepared_hot_is_suffix7_no_cross();
void single_hash_fast_prepared_hot(const uint8_t *input, uint8_t *output);
void single_hash_fast_prepared_hot_nonce32(const uint8_t *input, uint32_t nonce32, uint8_t *output);
void single_hash_fast_prepared_hot_nonce32_rolling(const uint8_t *input, uint32_t nonce32, uint8_t *output);
void single_hash_fast_prepared_hot_nonce32_dynamic_no_cross(uint32_t nonce32, uint8_t *output);
void single_hash_fast_prepared_hot_nonce32_dynamic_cross(uint32_t nonce32, uint8_t *output);
void single_hash_fast_prepared(const uint8_t *input, size_t size, uint8_t *output, uint64_t cacheToken);
void single_hash_fast_prepared_unchecked(const uint8_t *input, size_t size, uint8_t *output);
void single_hash_fast(const uint8_t *input, size_t size, uint8_t *output, uint64_t cacheToken);

template<Algorithm::Id ALGO>
void single_hash(const uint8_t *input, size_t size, uint8_t *output, cryptonight_ctx **ctx, uint64_t height);

// Explicit specialization used by CnHash for VerusHash.
template<>
void single_hash<Algorithm::VERUSHASH>(const uint8_t *input, size_t size, uint8_t *output, cryptonight_ctx **ctx, uint64_t height);

} // namespace verushash
} // namespace xmrig


#endif /* XMRIG_VERUSHASH_XMRIG_H */

