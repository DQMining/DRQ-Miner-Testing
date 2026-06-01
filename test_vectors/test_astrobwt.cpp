#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <chrono>
#include <memory>
#include <thread>
#include <atomic>
#include <vector>

#include "../src/crypto/astrobwt/AstroBWT.h"

static void hex_encode(const uint8_t* data, int len, char* out) {
    for (int i = 0; i < len; i++) {
        sprintf(out + i * 2, "%02x", data[i]);
    }
    out[len * 2] = '\0';
}

static int hex_decode(const char* hex, uint8_t* out) {
    int len = (int)strlen(hex) / 2;
    for (int i = 0; i < len; i++) {
        unsigned int val;
        sscanf(hex + i * 2, "%02x", &val);
        out[i] = (uint8_t)val;
    }
    return len;
}

struct TestCase {
    const char* input_hex;
    const char* expected_hash;
};

static TestCase tests[] = {
    { "", "b169fe67557f5c5e90087deaa9b3e30c4b1d36328a30b6a57e78a6124c477785" },
    { "00", "af73c0d88971f11ae18f7be731464c2d26e1a5610d601ab689d3c50212d1f2ca" },
    { "01020304", "2ae6df191d992ca9c777c819b9c9bb1e5c4c6052311e3e0ebb63e9cf29ca4f98" },
    { "48656c6c6f2c204445524f21", "793851405d33aefa2478e0473c2f1636fa3bee5733267d7498ba03964f1c279d" },
    { "7465737420696e70757420666f7220617374726f627774207633", "5e3c34197fd085996799f5d9b00f4adf7dc221c542cfddb8c4af792c20b4577b" },
    { "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f202122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f404142434445464748494a4b", "8cd2365ad0ba2eead832d4471f04fad3260e41984be85e2b7c73ba8ba94c9a15" },
};

int main(int argc, char** argv) {
    xmrig::astrobwt::init();

    const size_t scratch_size = 1024 * 1024 + 256;
    auto scratchpad = std::make_unique<uint8_t[]>(scratch_size);
    uint8_t input[256];
    uint8_t output[32];
    char hex_out[65];

    if (argc > 1 && strcmp(argv[1], "--bench-mt") == 0) {
        const int threads = (argc > 2) ? atoi(argv[2]) : 32;
        const int iterations = (argc > 3) ? atoi(argv[3]) : 200;
        int input_len = hex_decode("01020304", input);

        std::vector<std::thread> workers;
        std::atomic<int> done{0};
        const auto t0 = std::chrono::steady_clock::now();

        for (int t = 0; t < threads; t++) {
            workers.emplace_back([&]() {
                auto local_scratch = std::make_unique<uint8_t[]>(scratch_size);
                uint8_t local_out[32];
                for (int i = 0; i < iterations; i++) {
                    xmrig::astrobwt::astrobwt_dero_v3(input, input_len, local_scratch.get(), local_out);
                }
                done.fetch_add(iterations, std::memory_order_relaxed);
            });
        }
        for (auto& w : workers) w.join();
        const auto t1 = std::chrono::steady_clock::now();

        const double sec = std::chrono::duration<double>(t1 - t0).count();
        const int total = done.load();
        const double hps = total / sec;
        printf("AstroBWT v3 MT bench: %d threads, %d hashes in %.3f s => %.1f H/s (%.2f kH/s)\n",
               threads, total, sec, hps, hps / 1000.0);
        return 0;
    }

    if (argc > 1 && strcmp(argv[1], "--bench") == 0) {
        const int iterations = (argc > 2) ? atoi(argv[2]) : 500;
        int input_len = hex_decode("01020304", input);

        for (int warmup = 0; warmup < 20; warmup++) {
            xmrig::astrobwt::astrobwt_dero_v3(input, input_len, scratchpad.get(), output);
        }

        const auto t0 = std::chrono::steady_clock::now();
        for (int i = 0; i < iterations; i++) {
            xmrig::astrobwt::astrobwt_dero_v3(input, input_len, scratchpad.get(), output);
        }
        const auto t1 = std::chrono::steady_clock::now();

        const double sec = std::chrono::duration<double>(t1 - t0).count();
        const double hps = iterations / sec;
        printf("AstroBWT v3 bench: %d hashes in %.3f s => %.1f H/s (%.2f kH/s)\n",
               iterations, sec, hps, hps / 1000.0);
        return 0;
    }

    int passed = 0;
    int failed = 0;
    int total = sizeof(tests) / sizeof(tests[0]);

    for (int i = 0; i < total; i++) {
        int input_len = hex_decode(tests[i].input_hex, input);

        bool ok = xmrig::astrobwt::astrobwt_dero_v3(input, input_len, scratchpad.get(), output);
        hex_encode(output, 32, hex_out);

        if (!ok) {
            printf("Test %d: FAILED (astrobwt_dero_v3 returned false)\n", i);
            failed++;
            continue;
        }

        if (strcmp(hex_out, tests[i].expected_hash) == 0) {
            printf("Test %d: PASSED\n", i);
            passed++;
        } else {
            printf("Test %d: FAILED\n", i);
            printf("  Expected: %s\n", tests[i].expected_hash);
            printf("  Got:      %s\n", hex_out);
            failed++;
        }
    }

    printf("\n%d/%d tests passed, %d failed\n", passed, total, failed);
    return failed > 0 ? 1 : 0;
}
