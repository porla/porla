#pragma once

#include "../method.hpp"
#include "sessionssettingsget_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods::Sessions
{
    class SessionsSettingsGet : public Method<SessionsSettingsGetReq, SessionsSettingsGetRes>
    {
    public:
        explicit SessionsSettingsGet(porla::Sessions& sessions);

    protected:
        void Invoke(const SessionsSettingsGetReq& req, WriteCb<SessionsSettingsGetRes> cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
