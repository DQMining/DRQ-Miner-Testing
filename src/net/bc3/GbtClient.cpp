/* BitcoinIII (BC3) — getblocktemplate HTTP polling stub. */

#include "net/bc3/GbtClient.h"

#include "3rdparty/rapidjson/document.h"
#include "base/io/json/Json.h"
#include "base/io/json/JsonRequest.h"
#include "base/io/log/Log.h"
#include "base/io/log/Tags.h"
#include "base/net/http/Fetch.h"
#include "base/net/http/HttpData.h"
#include "base/net/http/HttpListener.h"
#include "base/net/stratum/Pool.h"

#include <cinttypes>
#ifdef XMRIG_OS_WIN
#   define strcasecmp _stricmp
#endif

#include <cstring>


namespace xmrig {


static const char *kBc3AlgoPlaceholder = "sha3-256t";


namespace {


class GbtHttpListener : public IHttpListener
{
public:
    void onHttpData(const HttpData &data) override
    {
        if (data.status != 200) {
            LOG_WARN("%s BC3 GBT poll HTTP %d", Tags::network(), data.status);
            return;
        }

        rapidjson::Document doc;
        if (doc.Parse(data.body.c_str()).HasParseError()) {
            LOG_WARN("%s BC3 GBT poll JSON parse failed", Tags::network());
            return;
        }

        const auto &result = doc["result"];
        if (result.IsObject() && result.HasMember("height")) {
            LOG_DEBUG("%s BC3 GBT template height %" PRIu64, Tags::network(), Json::getUint64(result, "height"));
        }
    }
};


} // namespace


const char *GbtClient::algoPlaceholder()
{
    return kBc3AlgoPlaceholder;
}


bool GbtClient::isBc3Pool(const Pool &pool)
{
    const char *algo = pool.algorithm().name();
    if (algo) {
        if (strcasecmp(algo, "bc3") == 0 || strcasecmp(algo, kBc3AlgoPlaceholder) == 0) {
            return true;
        }
    }

    const char *coin = pool.coin().code();
    return coin && strcasecmp(coin, "bc3") == 0;
}


void GbtClient::bootstrap(const Pool &pool)
{
    if (!isBc3Pool(pool)) {
        return;
    }

    LOG_NOTICE("%s " WHITE_BOLD("BC3 GBT client ready") " (algo placeholder: %s)", Tags::network(), kBc3AlgoPlaceholder);
}


GbtClient::GbtClient(const Pool &pool) :
    m_pool(pool)
{
    bootstrap(pool);
    m_ready = true;
}


GbtClient::~GbtClient() = default;


void GbtClient::poll()
{
    if (!m_ready) {
        return;
    }

    requestBlockTemplate();
}


void GbtClient::requestBlockTemplate()
{
    using namespace rapidjson;

    Document doc(kObjectType);
    auto &allocator = doc.GetAllocator();

    Value params(kObjectType);
    params.AddMember("rules", Value(kArrayType), allocator);
    params.AddMember("capabilities", Value(kArrayType), allocator);

    JsonRequest::create(doc, 1, "getblocktemplate", params);

    FetchRequest req(HTTP_POST, m_pool.host(), m_pool.port(), String("/"), doc, m_pool.isTLS(), true);

    fetch(Tags::network(), std::move(req), std::make_shared<GbtHttpListener>());
}


} // namespace xmrig
