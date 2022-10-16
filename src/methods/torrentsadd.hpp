#pragma once

#include "method.hpp"
#include "torrentsaddreq.hpp"
#include "torrentsaddres.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Methods
{
    class TorrentsAdd : public Method<TorrentsAddReq, TorrentsAddRes>
    {
    public:
        explicit TorrentsAdd(ISession& session);

    protected:
        void Invoke(const TorrentsAddReq& req, WriteCb<TorrentsAddRes> cb) override;

    private:
        ISession& m_session;
    };
}
