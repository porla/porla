#pragma once

#include <sqlite3.h>

#include "method.hpp"
#include "sessionsettingsupdate_reqres.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Methods
{
    class SessionSettingsUpdate : public Method<SessionSettingsUpdateReq, SessionSettingsUpdateRes>
    {
    public:
        explicit SessionSettingsUpdate(ISession& session, sqlite3* db);

    protected:
        void Invoke(const SessionSettingsUpdateReq& req, WriteCb<SessionSettingsUpdateRes> cb) override;

    private:
        ISession& m_session;
        sqlite3* m_db;
    };
}
