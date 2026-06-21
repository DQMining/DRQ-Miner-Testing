#ifndef XMRIG_DRQ_SETUP_H
#define XMRIG_DRQ_SETUP_H


namespace xmrig {


class Process;


class DrqSetup
{
public:
    enum Mode {
        Auto,
        Force
    };

    /** @return 0 ok/skip, 1 cancelled, 2 error */
    static int run(const Process &process, Mode mode);
};


} // namespace xmrig


#endif /* XMRIG_DRQ_SETUP_H */
