/* DRQ Miner — auto-update configuration. */

#include "core/update/UpdateConfig.h"

#include "3rdparty/rapidjson/document.h"
#include "base/io/json/Json.h"
#include "base/kernel/interfaces/IJsonReader.h"


namespace xmrig {


const char *UpdateConfig::kAutoUpdate     = "auto-update";
const char *UpdateConfig::kCheckInterval  = "update-check-interval";
const char *UpdateConfig::kManifestUrl    = "update-manifest-url";


void UpdateConfig::loadCli(bool applyUpdate)
{
    m_applyUpdate = applyUpdate;
    if (applyUpdate) {
        m_mode = Install;
    }
}


void UpdateConfig::load(const IJsonReader &reader)
{
    const auto &value = reader.getValue(kAutoUpdate);
    if (value.IsBool()) {
        m_mode = value.GetBool() ? Notify : Off;
    }
    else if (value.IsString()) {
        const char *s = value.GetString();
        if (strcmp(s, "install") == 0) {
            m_mode = Install;
        }
        else if (strcmp(s, "notify") == 0) {
            m_mode = Notify;
        }
        else if (strcmp(s, "false") == 0 || strcmp(s, "off") == 0) {
            m_mode = Off;
        }
        else {
            m_mode = Notify;
        }
    }

    const char *manifest = reader.getString(kManifestUrl);
    if (manifest) {
        m_manifestUrl = manifest;
    }
}


} // namespace xmrig
