#pragma once

#include "method.hpp"
#include "sessionsettingslist_reqres.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Methods
{
    class SessionSettingsList : public Method<SessionSettingsListReq, SessionSettingsListRes>
    {
    public:
        explicit SessionSettingsList(ISession& session);

    protected:
        void Invoke(const SessionSettingsListReq& req, WriteCb<SessionSettingsListRes> cb) override;

    private:
        ISession& m_session;
    };
}
