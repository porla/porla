#pragma once

#include "../method.hpp"
#include "sessionspause_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class SessionsPause : public Method<SessionsPauseReq, SessionsPauseRes>
    {
    public:
        explicit SessionsPause(porla::Sessions& sessions);

    protected:
        void Invoke(const SessionsPauseReq& req, WriteCb<SessionsPauseRes> cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
