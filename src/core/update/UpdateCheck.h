/* DRQ Miner — fetch manifest and compare versions. */

#ifndef XMRIG_UPDATECHECK_H
#define XMRIG_UPDATECHECK_H


#include "base/kernel/interfaces/IHttpListener.h"
#include "base/kernel/interfaces/IDnsListener.h"
#include "base/kernel/interfaces/ITimerListener.h"
#include "base/tools/Object.h"
#include "base/tools/String.h"
#include "core/update/UpdateConfig.h"


#include <memory>


namespace xmrig {


class Controller;
class HttpListener;
class Timer;


class UpdateCheck : public IHttpListener, public IDnsListener, public ITimerListener
{
public:
    XMRIG_DISABLE_COPY_MOVE(UpdateCheck)

    UpdateCheck(Controller *controller, const UpdateConfig &config);
    ~UpdateCheck() override;

    void start();
    void checkNow();

    static int compareVersion(const char *local, const char *remote);
    static String artifactKey();

protected:
    void onHttpData(const HttpData &data) override;
    void onResolved(const DnsRecords &records, int status, const char *error) override;
    void onTimer(const Timer *timer) override;

private:
    void fetchManifest();
    void handleManifest(const char *body, size_t size);
    void scheduleNext();

    Controller *m_controller;
    UpdateConfig m_config;
    std::shared_ptr<HttpListener> m_httpListener;
    String m_host;
    String m_path;
    uint16_t m_port   = 443;
    bool m_tls        = true;
    bool m_busy       = false;
    Timer *m_timer    = nullptr;
};


} // namespace xmrig


#endif // XMRIG_UPDATECHECK_H
