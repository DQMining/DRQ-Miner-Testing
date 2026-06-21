/* DRQ Miner — fetch manifest and compare versions. */

#include "core/update/UpdateCheck.h"

#include "3rdparty/rapidjson/document.h"
#include "base/io/json/Json.h"
#include "base/io/log/Log.h"
#include "base/io/log/Tags.h"
#include "base/net/dns/Dns.h"
#include "base/net/dns/DnsRecords.h"
#include "base/net/http/Fetch.h"
#include "base/net/http/HttpData.h"
#include "base/net/http/HttpListener.h"
#include "base/tools/Timer.h"
#include "core/Controller.h"
#include "core/update/UpdateApply.h"
#include "version.h"


#ifdef XMRIG_OS_WIN
#   include "base/kernel/Platform.h"
#endif


#include <cstdio>
#include <cstring>


namespace xmrig {


static const char *kTag = "update";


static bool parseManifestUrl(const String &url, String &host, String &path, uint16_t &port, bool &tls)
{
    const char *p = url.data();
    if (strncmp(p, "https://", 8) == 0) {
        tls  = true;
        port = 443;
        p   += 8;
    }
    else if (strncmp(p, "http://", 7) == 0) {
        tls  = false;
        port = 80;
        p   += 7;
    }
    else {
        return false;
    }

    const char *slash = strchr(p, '/');
    if (slash == nullptr) {
        host = p;
        path = "/";
    }
    else {
        host = String(p, slash - p);
        path = slash;
    }

    const char *colon = strchr(host.data(), ':');
    if (colon != nullptr && colon < host.data() + host.size()) {
        const long customPort = strtol(colon + 1, nullptr, 10);
        if (customPort > 0 && customPort <= 65535) {
            port = static_cast<uint16_t>(customPort);
        }
        host = String(host.data(), colon - host.data());
    }

    return !host.isNull();
}


UpdateCheck::UpdateCheck(Controller *controller, const UpdateConfig &config) :
    m_controller(controller),
    m_config(config)
{
    m_timer = new Timer(this);
}


UpdateCheck::~UpdateCheck()
{
    delete m_timer;
}


void UpdateCheck::start()
{
    if (!m_config.enabled()) {
        return;
    }

    if (!parseManifestUrl(m_config.manifestUrl(), m_host, m_path, m_port, m_tls)) {
        LOG_WARN("%s " YELLOW("invalid update-manifest-url: \"%s\""), Tags::network(), m_config.manifestUrl().data());
        return;
    }

    checkNow();
}


void UpdateCheck::checkNow()
{
    if (m_busy || !m_config.enabled()) {
        return;
    }

    m_busy = true;
    m_httpListener.reset();
    m_httpListener = std::make_shared<HttpListener>(this, kTag);
    Dns::resolve(m_host, this);
}


void UpdateCheck::scheduleNext()
{
    if (m_config.checkInterval() == 0) {
        return;
    }

    m_timer->start(m_config.checkInterval() * 1000ULL, 0);
}


void UpdateCheck::onTimer(const Timer *)
{
    checkNow();
}


void UpdateCheck::onResolved(const DnsRecords &, int status, const char *error)
{
    if (status < 0) {
        LOG_WARN("%s " YELLOW("update check DNS failed: \"%s\""), Tags::network(), error ? error : "unknown");
        m_busy = false;
        scheduleNext();
        return;
    }

    FetchRequest req(HTTP_GET, m_host, m_port, m_path, m_tls, false);
    fetch(kTag, std::move(req), m_httpListener);
}


void UpdateCheck::onHttpData(const HttpData &data)
{
    m_busy = false;

    if (data.status != 200) {
        LOG_WARN("%s " YELLOW("update manifest HTTP %d"), Tags::network(), data.status);
        scheduleNext();
        return;
    }

    handleManifest(data.body.c_str(), data.body.size());
    scheduleNext();
}


int UpdateCheck::compareVersion(const char *local, const char *remote)
{
    unsigned la = 0, lb = 0, lc = 0;
    unsigned ra = 0, rb = 0, rc = 0;

    sscanf(local, "%u.%u.%u", &la, &lb, &lc);
    sscanf(remote, "%u.%u.%u", &ra, &rb, &rc);

    if (ra != la) {
        return ra > la ? 1 : -1;
    }
    if (rb != lb) {
        return rb > lb ? 1 : -1;
    }
    if (rc != lc) {
        return rc > lc ? 1 : -1;
    }

    return 0;
}


String UpdateCheck::artifactKey()
{
#if defined(XMRIG_OS_WIN)
    return "win64";
#elif defined(XMRIG_OS_LINUX) && (defined(__aarch64__) || defined(__arm64__))
    return "linux-arm64-phone";
#elif defined(XMRIG_OS_LINUX)
    return "linux-x64";
#else
    return "linux-x64";
#endif
}


void UpdateCheck::handleManifest(const char *body, size_t)
{
    rapidjson::Document doc;
    if (doc.Parse(body).HasParseError()) {
        LOG_WARN("%s " YELLOW("update manifest JSON parse error"), Tags::network());
        return;
    }

    const char *remoteVersion = Json::getString(doc, "version");
    if (!remoteVersion) {
        LOG_WARN("%s " YELLOW("update manifest missing version"), Tags::network());
        return;
    }

    const int cmp = compareVersion(APP_VERSION, remoteVersion);
    if (cmp >= 0) {
        LOG_INFO("%s " WHITE_BOLD("up to date") " (%s)", Tags::network(), APP_VERSION);
        return;
    }

    const auto key = artifactKey();
    const rapidjson::Value &artifacts = doc["artifacts"];
    if (!artifacts.IsObject() || !artifacts.HasMember(key.data())) {
        LOG_NOTICE("%s " CYAN_BOLD("update available") " %s -> %s (no artifact for %s)",
                   Tags::network(), APP_VERSION, remoteVersion, key.data());
        return;
    }

    const rapidjson::Value &entry = artifacts[key.data()];
    const char *url    = Json::getString(entry, "url");
    const char *sha256 = Json::getString(entry, "sha256");

    if (!url || !sha256) {
        LOG_NOTICE("%s " CYAN_BOLD("update available") " %s -> %s (artifact incomplete)",
                   Tags::network(), APP_VERSION, remoteVersion);
        return;
    }

    LOG_NOTICE("%s " CYAN_BOLD("update available") " %s -> %s", Tags::network(), APP_VERSION, remoteVersion);
    LOG_NOTICE("%s " WHITE_BOLD("download") " %s", Tags::network(), url);
    LOG_NOTICE("%s " WHITE_BOLD("sha256") " %s", Tags::network(), sha256);

    if (m_config.installOnCheck() || m_config.applyUpdateCli()) {
        UpdateApply::run(url, sha256, remoteVersion);
    }
}


} // namespace xmrig
