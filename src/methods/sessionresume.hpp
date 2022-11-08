#pragma once

#include "method.hpp"
#include "sessionresume_reqres.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Methods
{
    class SessionResume : public Method<SessionResumeReq, SessionResumeRes>
    {
    public:
        explicit SessionResume(ISession& session);

    protected:
        void Invoke(const SessionResumeReq& req, WriteCb<SessionResumeRes> cb) override;

    private:
        ISession& m_session;
    };
}
