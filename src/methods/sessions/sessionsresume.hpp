#pragma once

#include "../method.hpp"
#include "sessionsresume_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class SessionsResume : public Method<SessionsResumeReq, SessionsResumeRes>
    {
    public:
        explicit SessionsResume(Sessions& sessions);

    protected:
        void Invoke(const SessionsResumeReq& req, WriteCb<SessionsResumeRes> cb) override;

    private:
        Sessions& m_sessions;
    };
}
