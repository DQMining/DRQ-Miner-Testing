/* DRQ Miner — auto-update configuration. */

#ifndef XMRIG_UPDATECONFIG_H
#define XMRIG_UPDATECONFIG_H


#include "base/tools/String.h"


namespace xmrig {


class IJsonReader;


class UpdateConfig
{
public:
    static const char *kAutoUpdate;
    static const char *kCheckInterval;
    static const char *kManifestUrl;

    enum Mode {
        Off,
        Notify,
        Install
    };

    UpdateConfig() = default;

    void load(const IJsonReader &reader);
    void loadCli(bool applyUpdate);

    inline Mode mode() const { return m_mode; }
    inline bool enabled() const { return m_mode != Off; }
    inline bool installOnCheck() const { return m_mode == Install || m_applyUpdate; }
    inline bool applyUpdateCli() const { return m_applyUpdate; }
    inline uint32_t checkInterval() const { return m_checkInterval; }
    inline const String &manifestUrl() const { return m_manifestUrl; }

private:
    Mode m_mode           = Notify;
    bool m_applyUpdate    = false;
    uint32_t m_checkInterval = 86400U;
    String m_manifestUrl  = "https://www.dqmining.com/releases/manifest.json";
};


} // namespace xmrig


#endif // XMRIG_UPDATECONFIG_H
