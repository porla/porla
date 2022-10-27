#pragma once

#include "method.hpp"
#include "torrentsget_reqres.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Methods
{
    class TorrentsGet : public Method<TorrentsGetReq, TorrentsGetRes>
    {
    public:
        explicit TorrentsGet(porla::ISession& session);

        void Invoke(const TorrentsGetReq& req, WriteCb<TorrentsGetRes> cb) override;

    private:
        porla::ISession& m_session;
    };
}
