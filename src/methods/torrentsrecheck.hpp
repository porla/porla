#pragma once

#include "method.hpp"
#include "torrentsrecheck_reqres.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Methods
{
    class TorrentsRecheck : public Method<TorrentsRecheckReq, TorrentsRecheckRes>
    {
    public:
        explicit TorrentsRecheck(ISession& session);

    protected:
        void Invoke(const TorrentsRecheckReq& req, WriteCb<TorrentsRecheckRes> cb) override;

    private:
        ISession& m_session;
    };
}
