/* Self-test for BC3 SHA3-256t (triple NIST SHA3-256). */

#include <cstdio>
#include <cstdint>
#include <cstring>

#include "base/crypto/sha3.h"

static int fail(const char *msg)
{
    std::fprintf(stderr, "FAIL: %s\n", msg);
    return 1;
}

static int expect_eq(const uint8_t *got, const uint8_t *want, size_t n, const char *label)
{
    if (std::memcmp(got, want, n) != 0) {
        std::fprintf(stderr, "FAIL: %s mismatch\n", label);
        return 1;
    }
    return 0;
}

static sha3_return_t sha3_256_chain(const void *in, unsigned inBytes, void *out32)
{
    uint8_t buf[32];

    if (sha3_HashBuffer(256, SHA3_FLAGS_NONE, in, inBytes, buf, 32) != SHA3_RETURN_OK) {
        return SHA3_RETURN_BAD_PARAMS;
    }
    if (sha3_HashBuffer(256, SHA3_FLAGS_NONE, buf, 32, buf, 32) != SHA3_RETURN_OK) {
        return SHA3_RETURN_BAD_PARAMS;
    }
    return sha3_HashBuffer(256, SHA3_FLAGS_NONE, buf, 32, out32, 32);
}

int main()
{
    uint8_t hashOut[32];
    uint8_t hashRef[32];
    uint8_t blockHdr[80];
    uint8_t hashHdr[32];

    std::memset(blockHdr, 0, sizeof(blockHdr));

    if (sha3_256t("", 0, hashOut) != SHA3_RETURN_OK) {
        return fail("sha3_256t empty");
    }
    if (sha3_256_chain("", 0, hashRef) != SHA3_RETURN_OK) {
        return fail("sha3_256_chain empty");
    }
    if (expect_eq(hashOut, hashRef, 32, "empty triple")) {
        return 1;
    }

    if (sha3_256t(blockHdr, 80, hashOut) != SHA3_RETURN_OK) {
        return fail("sha3_256t 80");
    }
    if (sha3_256_chain(blockHdr, 80, hashRef) != SHA3_RETURN_OK) {
        return fail("sha3_256_chain 80");
    }
    if (expect_eq(hashOut, hashRef, 32, "80-byte triple")) {
        return 1;
    }

    if (sha3_256t_header80(blockHdr, hashHdr) != SHA3_RETURN_OK) {
        return fail("sha3_256t_header80");
    }
    if (expect_eq(hashHdr, hashOut, 32, "header80 helper")) {
        return 1;
    }

    std::printf("test_sha3_256t: OK\n");
    return 0;
}
