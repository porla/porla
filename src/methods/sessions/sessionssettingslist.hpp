#pragma once

#include "../method.hpp"
#include "sessionssettingslist_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class SessionsSettingsList : public Method<SessionsSettingsListReq, SessionsSettingsListRes>
    {
    public:
        explicit SessionsSettingsList(Sessions& sessions);

    protected:
        void Invoke(const SessionsSettingsListReq& req, WriteCb<SessionsSettingsListRes> cb) override;

    private:
        Sessions& m_sessions;
    };
}
