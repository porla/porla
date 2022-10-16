#pragma once

#include "method.hpp"
#include "torrentsremovereq.hpp"
#include "torrentsremoveres.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Methods
{
    class TorrentsRemove : public Method<TorrentsRemoveReq, TorrentsRemoveRes>
    {
    public:
        explicit TorrentsRemove(ISession& session);

    protected:
        void Invoke(const TorrentsRemoveReq& req, WriteCb<TorrentsRemoveRes> cb) override;

    private:
        ISession& m_session;
    };
}
