#pragma once

#include "method.hpp"
#include "sessionsettingslist_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class SessionSettingsList : public Method<SessionSettingsListReq, SessionSettingsListRes>
    {
    public:
        explicit SessionSettingsList(Sessions& sessions);

    protected:
        void Invoke(const SessionSettingsListReq& req, WriteCb<SessionSettingsListRes> cb) override;

    private:
        Sessions& m_sessions;
    };
}
