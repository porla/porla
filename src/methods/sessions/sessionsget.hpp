#pragma once

#include <sqlite3.h>

#include "../method.hpp"
#include "sessionsget_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods::Sessions
{
    class SessionsGet : public Method<SessionsGetReq, SessionsGetRes>
    {
    public:
        explicit SessionsGet(sqlite3* db, porla::Sessions& sessions);

    protected:
        void Invoke(const SessionsGetReq& req, WriteCb<SessionsGetRes> cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
