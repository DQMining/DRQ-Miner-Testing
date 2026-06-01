#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <ctime>

#include "../src/crypto/astrobwt/AstroBWT.h"

int main() {
    xmrig::astrobwt::init();

    uint8_t blob[76];
    memset(blob, 0, sizeof(blob));
    blob[0] = 0x01;
    blob[1] = 0x02;

    uint8_t scratchpad[1024 * 1024 + 256];
    uint8_t hash[32];

    uint64_t target = 0xFFFFFFFFFFFFFFFFULL / 20000;
    printf("Target: %016llx\n", (unsigned long long)target);

    int found = 0;
    int total = 10000;

    srand((unsigned)time(NULL));

    for (int i = 0; i < total; i++) {
        uint32_t nonce = (uint32_t)i;
        memcpy(blob + 39, &nonce, 4);

        xmrig::astrobwt::astrobwt_dero_v3(blob, 76, scratchpad, hash);

        uint64_t value = *reinterpret_cast<uint64_t*>(hash + 24);

        if (value < target) {
            found++;
            if (found <= 5) {
                printf("Share found at nonce %u: hash[24:32]=", nonce);
                for (int j = 24; j < 32; j++) printf("%02x", hash[j]);
                printf(" value=%016llx\n", (unsigned long long)value);
            }
        }
    }

    printf("\nFound %d shares in %d hashes (expected ~%d at diff 20000)\n", found, total, total / 20000 + 1);
    return 0;
}
