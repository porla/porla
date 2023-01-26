#pragma once

#include "method.hpp"
#include "torrentsmediainfo_reqres.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Methods
{
    class TorrentsMediaInfo : public Method<TorrentsMediaInfoReq, TorrentsMediaInfoRes>
    {
    public:
        explicit TorrentsMediaInfo(ISession& session);

    protected:
        void Invoke(const TorrentsMediaInfoReq& req, WriteCb<TorrentsMediaInfoRes> cb) override;

    private:
        ISession& m_session;
    };
}
