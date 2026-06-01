#include <cstdio>
#include <cstdint>
#include <cstring>

#include "../src/crypto/astrobwt/sha256_utils.h"
#include "../src/crypto/astrobwt/hash_utils.h"
#include "../src/crypto/astrobwt/sais.h"
#include "../src/crypto/astrobwt/Salsa20.hpp"
#include "../src/crypto/astrobwt/AstroBWT.h"

static void hex_print(const char* label, const uint8_t* data, int len) {
    printf("%s: ", label);
    for (int i = 0; i < len; i++) printf("%02x", data[i]);
    printf("\n");
}

bool debug_astrobwt_v3(const uint8_t* input_data, uint32_t input_size, uint8_t* output_hash) {
    using namespace xmrig::astrobwt;

    static uint8_t scratch_mem[1024 * 1024 * 2];
    auto* scratch = reinterpret_cast<ScratchData*>(scratch_mem);

    uint8_t step_3[256];
    memset(step_3, 0, sizeof(step_3));

    uint8_t sha_key[32];
    sha256(input_data, input_size, sha_key);

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

        if (tries <= 5 || (tries >= 258 && tries <= 280)) {
            printf("C++ Iter %llu: op=%d pos1=%d pos2=%d lhash=%016llx prev=%016llx\n",
                (unsigned long long)tries, op, pos1, pos2,
                (unsigned long long)lhash, (unsigned long long)prev_lhash);
            printf("  s[0:16]=");
            for (int j = 0; j < 16; j++) printf("%02x", s[j]);
            printf(" s[255]=%02x\n", s[255]);
        }

#include "../src/crypto/astrobwt/astrobwt_ops.inl"

        if (tries <= 5 || (tries >= 258 && tries <= 280)) {
            printf("  after ops s[0:16]=");
            for (int j = 0; j < 16; j++) printf("%02x", s[j]);
            printf("\n");
        }

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

        memcpy(scratch->data + (tries - 1) * 256, s, 256);

        if (tries > 260 + 16 || (s[255] >= 0xf0 && tries > 260)) {
            printf("Loop ended at tries=%llu s[255]=%02x\n", (unsigned long long)tries, s[255]);
            break;
        }
    }

    uint32_t data_len = static_cast<uint32_t>(
        ((tries - 4) * 256) + (((static_cast<uint64_t>(s[253]) << 8) | static_cast<uint64_t>(s[254])) & 0x3ff)
    );
    printf("data_len=%u tries=%llu s[253]=%02x s[254]=%02x\n", data_len, (unsigned long long)tries, s[253], s[254]);

    if (data_len > MAX_LENGTH) {
        data_len = MAX_LENGTH;
    }

    memset(scratch->sa, 0, data_len * sizeof(int32_t));
    build_suffix_array(scratch->data, scratch->sa, static_cast<int32_t>(data_len));

    printf("SA[0:8]=");
    for (int i = 0; i < 8; i++) printf(" %d", scratch->sa[i]);
    printf("\n");

    sha256(reinterpret_cast<const uint8_t*>(scratch->sa), data_len * 4, output_hash);
    hex_print("Final hash", output_hash, 32);

    return true;
}

int main() {
    uint8_t input[] = {0x01, 0x02, 0x03, 0x04};
    uint8_t output[32];

    debug_astrobwt_v3(input, 4, output);
    return 0;
}
