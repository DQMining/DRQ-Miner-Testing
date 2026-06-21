#include "nm_sha256.h"

#include <openssl/sha.h>

void nm_sha256(const uint8_t *p, size_t len, uint8_t out[32])
{
    SHA256(p, len, out);
}
