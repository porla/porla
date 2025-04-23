#pragma once

#include "../method.hpp"
#include "sessionsresume_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    template <bool SSL> class SessionsResume : public Method<SessionsResumeReq, SessionsResumeRes, SSL>
    {
    public:
        explicit SessionsResume(Sessions& sessions);

    protected:
        void Invoke(const SessionsResumeReq& req, WriteCb<SessionsResumeRes, SSL> cb) override;

    private:
        Sessions& m_sessions;
    };
}
