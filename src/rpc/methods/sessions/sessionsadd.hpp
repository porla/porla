#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"
#include "sessionsadd_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Sessions
{
    class SessionsAdd : public TypedMethod<SessionsAddReq, SessionsAddRes>
    {
    public:
        explicit SessionsAdd(sqlite3* db, porla::Sessions& sessions);

        void Execute(const SessionsAddReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
