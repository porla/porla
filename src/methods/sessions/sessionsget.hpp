#pragma once

#include "../method.hpp"
#include "sessionsget_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods::Sessions
{
    class SessionsGet : public Method<SessionsGetReq, SessionsGetRes>
    {
    public:
        explicit SessionsGet(porla::Sessions& sessions);

    protected:
        void Invoke(const SessionsGetReq& req, WriteCb<SessionsGetRes> cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
