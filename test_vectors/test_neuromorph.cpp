/* Neuromorph (nm/1) smoke test — links ported Cereblix crypto. */

#include "crypto/nm/nm_neuromorph.h"
#include "crypto/nm/NmShared.h"

#include <cstdio>
#include <cstring>


int main()
{
    nm_ctx ctx{};
    if (nm_ctx_init(&ctx) != 0) {
        fprintf(stderr, "nm_ctx_init failed\n");
        return 1;
    }

    uint8_t seed[32];
    memset(seed, 0x42, sizeof(seed));
    nm_ctx_set_params(&ctx, seed);

    uint8_t key[16];
    const uint64_t *dataset = xmrig::NmShared::dataset(seed, 0, false, key);
    if (!dataset) {
        fprintf(stderr, "NmShared::dataset failed\n");
        nm_ctx_free(&ctx);
        return 1;
    }

    nm_ctx_attach_dataset(&ctx, const_cast<uint64_t *>(dataset), key);

    uint8_t header[124];
    memset(header, 0, sizeof(header));
    uint8_t hash[32];

    nm_hash(&ctx, header, 1, hash);

    printf("neuromorph smoke hash: ");
    for (int i = 0; i < 32; ++i) {
        printf("%02x", hash[i]);
    }
    printf("\n");

    nm_ctx_free(&ctx);
    return 0;
}
