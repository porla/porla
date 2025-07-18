#pragma once

#include "../method.hpp"
#include "sessionspause_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    template <bool SSL> class SessionsPause : public Method<SessionsPauseReq, SessionsPauseRes, SSL>
    {
    public:
        explicit SessionsPause(Sessions& sessions);

    protected:
        void Invoke(const SessionsPauseReq& req, WriteCb<SessionsPauseRes, SSL> cb) override;

    private:
        Sessions& m_sessions;
    };
}
