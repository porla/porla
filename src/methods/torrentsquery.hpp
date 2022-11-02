#pragma once

#include "method.hpp"
#include "torrentsquery_reqres.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Methods
{
    class TorrentsQuery : public Method<TorrentsQueryReq, TorrentsQueryRes>
    {
    public:
        explicit TorrentsQuery(ISession& session);

        void Invoke(const TorrentsQueryReq& req, WriteCb<TorrentsQueryRes> cb) override;

    private:
        ISession& m_session;
    };
}
