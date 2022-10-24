#pragma once

#include "method.hpp"
#include "torrentsresume_reqres.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Methods
{
    class TorrentsResume : public Method<TorrentsResumeReq, TorrentsResumeRes>
    {
    public:
        explicit TorrentsResume(ISession& session);

    protected:
        void Invoke(const TorrentsResumeReq& req, WriteCb<TorrentsResumeRes> cb) override;

    private:
        ISession& m_session;
    };
}
