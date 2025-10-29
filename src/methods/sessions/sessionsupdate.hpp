#pragma once

#include <sqlite3.h>

#include "../method.hpp"
#include "sessionsupdate_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods::Sessions
{
    class SessionsUpdate : public Method<SessionsUpdateReq, SessionsUpdateRes>
    {
    public:
        explicit SessionsUpdate(sqlite3* db, porla::Sessions& sessions);

        void Invoke(const SessionsUpdateReq& req, WriteCb<SessionsUpdateRes> cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
