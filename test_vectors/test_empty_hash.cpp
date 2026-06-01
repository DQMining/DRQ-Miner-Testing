#include <cstdio>
#include <cstdint>
#include "../src/crypto/astrobwt/hash_utils.h"

int main() {
    using namespace xmrig::astrobwt;
    uint8_t empty[1] = {0};
    printf("xxhash64(empty) = %016llx\n", (unsigned long long)xxhash64(empty, 0));
    printf("fnv1a(empty) = %016llx\n", (unsigned long long)fnv1a_hash64(empty, 0));
    printf("siphash(1, prev, empty) = %016llx\n", (unsigned long long)siphash(1, 0x293cbca5d2af413eULL, empty, 0));
    return 0;
}
