#pragma once

#include "method.hpp"
#include "sessionresume_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class SessionResume : public Method<SessionResumeReq, SessionResumeRes>
    {
    public:
        explicit SessionResume(Sessions& sessions);

    protected:
        void Invoke(const SessionResumeReq& req, WriteCb<SessionResumeRes> cb) override;

    private:
        Sessions& m_sessions;
    };
}
