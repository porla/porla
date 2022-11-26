#pragma once

#include "method.hpp"
#include "torrentsset_reqres.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Methods
{
    class TorrentsSet : public Method<TorrentsSetReq, TorrentsSetRes>
    {
    public:
        explicit TorrentsSet(ISession& session);

    protected:
        void Invoke(const TorrentsSetReq& req, WriteCb<TorrentsSetRes> cb) override;

    private:
        ISession& m_session;
    };
}
