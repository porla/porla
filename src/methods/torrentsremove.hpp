#pragma once

#include "method.hpp"
#include "torrentsremovereq.hpp"
#include "torrentsremoveres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    template <bool SSL> class TorrentsRemove : public Method<TorrentsRemoveReq, TorrentsRemoveRes, SSL>
    {
    public:
        explicit TorrentsRemove(Sessions& sessions);

    protected:
        void Invoke(const TorrentsRemoveReq& req, WriteCb<TorrentsRemoveRes, SSL> cb) override;

    private:
        Sessions& m_sessions;
    };
}
