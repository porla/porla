#pragma once

#include "method.hpp"
#include "sessionpause_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class SessionPause : public Method<SessionPauseReq, SessionPauseRes>
    {
    public:
        explicit SessionPause(Sessions& sessions);

    protected:
        void Invoke(const SessionPauseReq& req, WriteCb<SessionPauseRes> cb) override;

    private:
        Sessions& m_sessions;
    };
}
