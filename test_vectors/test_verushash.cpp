#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <thread>
#include <vector>

#include "crypto/verushash/verus_hash.h"

static const char kSelfTestInput[] =
    "Test1234Test1234Test1234Test1234"
    "Test1234Test1234Test1234Test1234"
    "Test1234Test1234Test1234Test1234";

static const char kSelfTestExpected[] =
    "ed3dbd1d798342264cbfee4a49564917edb68b3a5c566d1f487005113bc4ce55";

static void hex_encode(const uint8_t *data, size_t len, char *out)
{
    for (size_t i = 0; i < len; ++i) {
        std::sprintf(out + i * 2, "%02x", data[i]);
    }
    out[len * 2] = '\0';
}

static int run_self_test()
{
    CVerusHashV2::init();

    unsigned char hash[32] = {};
    verus_hash_v2(hash, kSelfTestInput, sizeof(kSelfTestInput) - 1);

    char hex[65];
    hex_encode(hash, sizeof(hash), hex);

    const bool ok = std::strcmp(hex, kSelfTestExpected) == 0;
    printf("VerusHash v2 self-test: %s\n", ok ? "PASSED" : "FAILED");
    if (!ok) {
        printf("  expected: %s\n", kSelfTestExpected);
        printf("  got     : %s\n", hex);
    }
    return ok ? 0 : 1;
}

static int run_bench_mt(int threads, int iterations)
{
    CVerusHashV2::init();

    std::vector<std::thread> workers;
    std::atomic<int> done{0};
    const auto t0 = std::chrono::steady_clock::now();

    for (int t = 0; t < threads; ++t) {
        workers.emplace_back([&, iterations]() {
            unsigned char hash[32];
            for (int i = 0; i < iterations; ++i) {
                verus_hash_v2(hash, kSelfTestInput, sizeof(kSelfTestInput) - 1);
            }
            done.fetch_add(iterations, std::memory_order_relaxed);
        });
    }
    for (auto &w : workers) {
        w.join();
    }

    const auto t1 = std::chrono::steady_clock::now();
    const double sec = std::chrono::duration<double>(t1 - t0).count();
    const int total = done.load();
    const double hps = total / sec;
    printf("VerusHash v2 MT bench: %d threads, %d hashes in %.3f s => %.1f H/s (%.2f kH/s)\n",
           threads, total, sec, hps, hps / 1000.0);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc > 1 && std::strcmp(argv[1], "--bench-mt") == 0) {
        const int threads = (argc > 2) ? std::atoi(argv[2]) : 32;
        const int iters   = (argc > 3) ? std::atoi(argv[3]) : 200;
        return run_bench_mt(threads, iters);
    }

    return run_self_test();
}
