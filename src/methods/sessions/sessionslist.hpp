#pragma once

#include <sqlite3.h>

#include "../method.hpp"
#include "sessionslist_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class SessionsList : public Method<SessionsListReq, SessionsListRes>
    {
    public:
        explicit SessionsList(sqlite3* db, porla::Sessions& sessions);

        void Invoke(const SessionsListReq& req, WriteCb<SessionsListRes> cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
