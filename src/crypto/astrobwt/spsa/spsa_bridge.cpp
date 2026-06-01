#include <cstring>

#include <windows.h>

#include <openssl/sha.h>

#include "astroworker.h"
#include "spsa.hpp"

namespace {

INIT_ONCE g_spsa_init_once = INIT_ONCE_STATIC_INIT;

BOOL CALLBACK init_spsa_once(PINIT_ONCE, PVOID, PVOID*)
{
    initSPSA();
    return TRUE;
}

workerData& thread_worker()
{
    thread_local workerData* worker = nullptr;
    if (!worker) {
        worker = new workerData();
    }
    return *worker;
}

} // namespace

extern "C" {

int xmrig_astro_spsa_available()
{
    return 1;
}

void xmrig_astro_spsa_init()
{
    InitOnceExecuteOnce(&g_spsa_init_once, init_spsa_once, nullptr, nullptr);
}

uint8_t* xmrig_astro_worker_sdata()
{
    InitOnceExecuteOnce(&g_spsa_init_once, init_spsa_once, nullptr, nullptr);
    return thread_worker().sData;
}

int xmrig_astro_spsa_finalize_with_templates(const uint8_t* data,
                                             int data_len,
                                             const templateMarker* templates,
                                             int template_count,
                                             uint8_t* output_hash)
{
    if (!data || data_len <= 0 || !output_hash) {
        return 0;
    }

    InitOnceExecuteOnce(&g_spsa_init_once, init_spsa_once, nullptr, nullptr);

    workerData& worker = thread_worker();

    if (data_len > static_cast<int>(ASTRO_SCRATCH_SIZE)) {
        data_len = static_cast<int>(ASTRO_SCRATCH_SIZE);
    }

    if (data != worker.sData) {
        std::memcpy(worker.sData, data, static_cast<size_t>(data_len));
    }
    worker.data_len = static_cast<uint32_t>(data_len);
    worker.templateIdx = 0;

    if (templates && template_count > 0) {
        if (template_count > 277) {
            template_count = 277;
        }
        std::memcpy(worker.astroTemplate, templates,
                    static_cast<size_t>(template_count) * sizeof(templateMarker));
        worker.templateIdx = template_count;
    }

    const bool already_sha = SPSA(worker.sData, data_len, worker);

    if (already_sha) {
        std::memcpy(output_hash, worker.padding, 32);
        return 1;
    }

    SHA256(reinterpret_cast<const uint8_t*>(worker.sa),
           static_cast<size_t>(data_len) * 4,
           output_hash);
    return 1;
}

int xmrig_astro_spsa_finalize(const uint8_t* data, int data_len, uint8_t* output_hash)
{
    return xmrig_astro_spsa_finalize_with_templates(data, data_len, nullptr, 0, output_hash);
}

} // extern "C"
