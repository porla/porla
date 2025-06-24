#pragma once

#include <sqlite3.h>

#include "../method.hpp"
#include "sessionsremove_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class SessionsRemove : public Method<SessionsRemoveReq, SessionsRemoveRes>
    {
    public:
        explicit SessionsRemove(sqlite3* db, porla::Sessions& sessions);

        void Invoke(const SessionsRemoveReq& req, WriteCb<SessionsRemoveRes> cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
