#pragma once

#include <sqlite3.h>

#include "../method.hpp"
#include "sessionspause_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class SessionsPause : public Method<SessionsPauseReq, SessionsPauseRes>
    {
    public:
        explicit SessionsPause(sqlite3* db, porla::Sessions& sessions);

    protected:
        void Invoke(const SessionsPauseReq& req, WriteCb<SessionsPauseRes> cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
