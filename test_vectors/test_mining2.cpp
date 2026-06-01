#include <cstdio>
#include <cstdint>
#include <cstring>

#include "../src/crypto/astrobwt/AstroBWT.h"

int main() {
    xmrig::astrobwt::init();

    uint8_t blob[76];
    memset(blob, 0, sizeof(blob));
    blob[0] = 0x01;
    blob[1] = 0x02;

    uint8_t scratchpad[1024 * 1024 + 256];
    uint8_t hash[32];

    uint64_t target_100 = 0xFFFFFFFFFFFFFFFFULL / 100;
    printf("Target (diff 100): %016llx\n", (unsigned long long)target_100);

    int found = 0;
    for (int i = 0; i < 1000; i++) {
        uint32_t nonce = (uint32_t)i;
        memcpy(blob + 39, &nonce, 4);

        xmrig::astrobwt::astrobwt_dero_v3(blob, 76, scratchpad, hash);

        uint64_t value = *reinterpret_cast<uint64_t*>(hash + 24);

        if (value < target_100) {
            found++;
            if (found <= 3) {
                printf("Share at nonce %u: value=%016llx < target=%016llx\n",
                    nonce, (unsigned long long)value, (unsigned long long)target_100);
            }
        }
    }

    printf("Found %d shares in 1000 hashes at diff 100 (expected ~10)\n", found);
    return 0;
}
