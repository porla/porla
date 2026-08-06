#pragma once

#include <sqlite3.h>

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
        explicit SessionsSettingsGet(sqlite3* db, porla::Sessions& sessions);

    protected:
        void Invoke(const SessionsSettingsGetReq& req, WriteCb<SessionsSettingsGetRes> cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
