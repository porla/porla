#pragma once

#include "method.hpp"
#include "torrentsoverview_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class TorrentsOverview : public Method<TorrentsOverviewReq, TorrentsOverviewRes>
    {
    public:
        explicit TorrentsOverview(porla::Sessions& sessions);

        void Invoke(const TorrentsOverviewReq& req, WriteCb<TorrentsOverviewRes> cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
