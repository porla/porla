#pragma once

#include "method.hpp"
#include "torrentsrecheck_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    template <bool SSL> class TorrentsRecheck : public Method<TorrentsRecheckReq, TorrentsRecheckRes, SSL>
    {
    public:
        explicit TorrentsRecheck(Sessions& sessions);

    protected:
        void Invoke(const TorrentsRecheckReq& req, WriteCb<TorrentsRecheckRes, SSL> cb) override;

    private:
        Sessions& m_sessions;
    };
}
