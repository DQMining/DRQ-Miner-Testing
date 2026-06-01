#include "crypto/astrobwt/spsa/spsa_finalize.h"

extern "C" {
int xmrig_astro_spsa_available();
void xmrig_astro_spsa_init();
uint8_t* xmrig_astro_worker_sdata();
int xmrig_astro_spsa_finalize(const uint8_t* data, int data_len, uint8_t* output_hash);
int xmrig_astro_spsa_finalize_with_templates(const uint8_t* data,
                                             int data_len,
                                             const xmrig::astrobwt::spsa::SpsaTemplateMarker* templates,
                                             int template_count,
                                             uint8_t* output_hash);
}

namespace xmrig {
namespace astrobwt {
namespace spsa {

bool available()
{
#if defined(XMRIG_ASTROBWT_USE_SPSA)
    return xmrig_astro_spsa_available() != 0;
#else
    return false;
#endif
}

void init()
{
#if defined(XMRIG_ASTROBWT_USE_SPSA)
    xmrig_astro_spsa_init();
#endif
}

uint8_t* worker_sdata()
{
#if defined(XMRIG_ASTROBWT_USE_SPSA)
    return xmrig_astro_worker_sdata();
#else
    return nullptr;
#endif
}

bool finalize(const uint8_t* data, int data_len, uint8_t* output_hash)
{
#if defined(XMRIG_ASTROBWT_USE_SPSA)
    return xmrig_astro_spsa_finalize(data, data_len, output_hash) != 0;
#else
    (void)data;
    (void)data_len;
    (void)output_hash;
    return false;
#endif
}

bool finalize_with_templates(const uint8_t* data,
                             int data_len,
                             const SpsaTemplateMarker* templates,
                             int template_count,
                             uint8_t* output_hash)
{
#if defined(XMRIG_ASTROBWT_USE_SPSA)
    return xmrig_astro_spsa_finalize_with_templates(data, data_len, templates, template_count, output_hash) != 0;
#else
    (void)data;
    (void)data_len;
    (void)templates;
    (void)template_count;
    (void)output_hash;
    return false;
#endif
}

} // namespace spsa
} // namespace astrobwt
} // namespace xmrig
