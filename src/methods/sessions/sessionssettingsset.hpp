#pragma once

#include "../method.hpp"
#include "sessionssettingsset_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods::Sessions
{
    class SessionsSettingsSet : public Method<SessionsSettingsSetReq, SessionsSettingsSetRes>
    {
    public:
        explicit SessionsSettingsSet(porla::Sessions& sessions);

    protected:
        void Invoke(const SessionsSettingsSetReq& req, WriteCb<SessionsSettingsSetRes> cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
