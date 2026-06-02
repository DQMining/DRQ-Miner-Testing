#include "crypto/astrobwt/astrobwt_suffix.h"

#include "crypto/astrobwt/AstroBWT.h"
#include "crypto/astrobwt/divsufsort.h"
#include "crypto/astrobwt/sa_fast.h"
#include "crypto/astrobwt/sha256_utils.h"
#include "crypto/astrobwt/spsa/spsa_finalize.h"

namespace xmrig {
namespace astrobwt {

void suffix_hash(uint8_t* data,
                 int32_t data_len,
                 ScratchData* scratch,
                 uint8_t* output_hash,
                 const spsa::SpsaTemplateMarker* templates,
                 int template_count)
{
#if defined(XMRIG_ASTROBWT_USE_SPSA)
    if (templates && template_count > 0
        && spsa::finalize_with_templates(data, data_len, templates, template_count, output_hash)) {
        return;
    }
    if (spsa::available() && spsa::finalize(data, data_len, output_hash)) {
        return;
    }
#endif

#if defined(XMRIG_ARM)
    build_suffix_array_fast_scratch(data, scratch->sa, data_len);
#else
    divsufsort(data, scratch->sa, data_len, scratch->bucket_a, scratch->bucket_b);
#endif
    sha256_auto(reinterpret_cast<const uint8_t*>(scratch->sa),
                static_cast<uint32_t>(data_len) * 4,
                output_hash);
}

} // namespace astrobwt
} // namespace xmrig
