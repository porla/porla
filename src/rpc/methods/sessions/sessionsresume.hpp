#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"
#include "sessionsresume_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Sessions
{
    class SessionsResume : public TypedMethod<SessionsResumeReq, SessionsResumeRes>
    {
    public:
        explicit SessionsResume(sqlite3* db, porla::Sessions& sessions);

    protected:
        void Execute(const SessionsResumeReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
