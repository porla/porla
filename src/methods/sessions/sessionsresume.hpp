#pragma once

#include <sqlite3.h>

#include "../method.hpp"
#include "sessionsresume_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class SessionsResume : public Method<SessionsResumeReq, SessionsResumeRes>
    {
    public:
        explicit SessionsResume(sqlite3* db, porla::Sessions& sessions);

    protected:
        void Invoke(const SessionsResumeReq& req, WriteCb<SessionsResumeRes> cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
