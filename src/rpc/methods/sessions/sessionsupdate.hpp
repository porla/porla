#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"
#include "sessionsupdate_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Sessions
{
    class SessionsUpdate : public TypedMethod<SessionsUpdateReq, SessionsUpdateRes>
    {
    public:
        explicit SessionsUpdate(sqlite3* db, porla::Sessions& sessions);

        void Execute(const SessionsUpdateReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
