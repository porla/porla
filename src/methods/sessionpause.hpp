#pragma once

#include "method.hpp"
#include "sessionpause_reqres.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Methods
{
    class SessionPause : public Method<SessionPauseReq, SessionPauseRes>
    {
    public:
        explicit SessionPause(ISession& session);

    protected:
        void Invoke(const SessionPauseReq& req, WriteCb<SessionPauseRes> cb) override;

    private:
        ISession& m_session;
    };
}
