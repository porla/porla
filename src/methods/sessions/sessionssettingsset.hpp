#pragma once

#include <sqlite3.h>

#include "../method.hpp"
#include "sessionssettingsset_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class SessionsSettingsSet : public Method<SessionsSettingsSetReq, SessionsSettingsSetRes>
    {
    public:
        explicit SessionsSettingsSet(sqlite3* db, porla::Sessions& sessions);

        void Invoke(const SessionsSettingsSetReq& req, WriteCb<SessionsSettingsSetRes> cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
