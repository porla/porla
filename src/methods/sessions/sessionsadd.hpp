#pragma once

#include <sqlite3.h>

#include "../method.hpp"
#include "sessionsadd_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class SessionsAdd : public Method<SessionsAddReq, SessionsAddRes>
    {
    public:
        explicit SessionsAdd(sqlite3* db, porla::Sessions& sessions);

        void Invoke(const SessionsAddReq& req, WriteCb<SessionsAddRes> cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
