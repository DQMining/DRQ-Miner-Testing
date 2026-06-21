/* DRQ Miner — download, verify SHA256, replace binary. */

#ifndef XMRIG_UPDATEAPPLY_H
#define XMRIG_UPDATEAPPLY_H


namespace xmrig {


class UpdateApply
{
public:
    static void run(const char *url, const char *expectedSha256, const char *version);
};


} // namespace xmrig


#endif // XMRIG_UPDATEAPPLY_H
