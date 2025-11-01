#pragma once

#include "../method.hpp"
#include "torrentscount_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods::Torrents
{
    class TorrentsCount : public Method<TorrentsCountReq, TorrentsCountRes>
    {
    public:
        explicit TorrentsCount(porla::Sessions& sessions);

    protected:
        void Invoke(const TorrentsCountReq& req, WriteCb<TorrentsCountRes> cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
