#pragma once

#include "../method.hpp"
#include "sessionssettingslist_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    template <bool SSL> class SessionsSettingsList : public Method<SessionsSettingsListReq, SessionsSettingsListRes, SSL>
    {
    public:
        explicit SessionsSettingsList(Sessions& sessions);

    protected:
        void Invoke(const SessionsSettingsListReq& req, WriteCb<SessionsSettingsListRes, SSL> cb) override;

    private:
        Sessions& m_sessions;
    };
}
