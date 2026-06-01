#include <cstdio>
#include <cstdint>
#include <cstring>

#include "../src/crypto/astrobwt/sha256_utils.h"
#include "../src/crypto/astrobwt/hash_utils.h"
#include "../src/crypto/astrobwt/Salsa20.hpp"

static void hex_print(const char* label, const uint8_t* data, int len) {
    printf("%s: ", label);
    for (int i = 0; i < len; i++) printf("%02x", data[i]);
    printf("\n");
}

int main() {
    using namespace xmrig::astrobwt;

    uint8_t input[] = {0x01, 0x02, 0x03, 0x04};
    uint32_t input_size = 4;

    printf("Input: 01020304\n");

    // Step 1: SHA-256
    uint8_t sha_key[32];
    sha256(input, input_size, sha_key);
    hex_print("SHA256 key", sha_key, 32);

    // Step 2: Salsa20
    uint8_t step_3[256];
    memset(step_3, 0, sizeof(step_3));
    const uint64_t iv = 0;
    ZeroTier::Salsa20 salsa(sha_key, &iv);
    salsa.XORKeyStream(step_3, 256);
    hex_print("After Salsa20 (first 64)", step_3, 64);
    hex_print("After Salsa20 (last 64)", step_3 + 192, 64);

    // Step 3: RC4
    RC4 rc4;
    rc4.init(step_3, 256);
    rc4.xor_stream(step_3, step_3, 256);
    hex_print("After RC4 (first 64)", step_3, 64);
    hex_print("After RC4 (last 64)", step_3 + 192, 64);

    // Step 4: FNV1a
    uint64_t lhash = fnv1a_hash64(step_3, 256);
    printf("FNV1a lhash: %016llx\n", (unsigned long long)lhash);

    // Step 5: First iterations
    uint64_t prev_lhash = lhash;
    uint64_t tries = 0;
    for (int iter = 0; iter < 5; iter++) {
        tries++;
        uint64_t random_switcher = prev_lhash ^ lhash ^ tries;
        uint8_t op = (uint8_t)(random_switcher);
        uint8_t pos1 = (uint8_t)(random_switcher >> 8);
        uint8_t pos2 = (uint8_t)(random_switcher >> 16);
        if (pos1 > pos2) { uint8_t t = pos1; pos1 = pos2; pos2 = t; }
        if ((uint8_t)(pos2 - pos1) > 32) {
            pos2 = pos1 + ((pos2 - pos1) & 0x1f);
        }
        printf("Iter %d: tries=%llu rs=%016llx op=%d pos1=%d pos2=%d\n",
            iter, (unsigned long long)tries, (unsigned long long)random_switcher, op, pos1, pos2);
        printf("  step_3[0:8] before: ");
        for (int j = 0; j < 8; j++) printf("%02x", step_3[j]);
        printf("\n");
    }

    return 0;
}
