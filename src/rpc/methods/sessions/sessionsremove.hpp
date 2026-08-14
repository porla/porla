#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"
#include "sessionsremove_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Sessions
{
    class SessionsRemove : public TypedMethod<SessionsRemoveReq, SessionsRemoveRes>
    {
    public:
        explicit SessionsRemove(sqlite3* db, porla::Sessions& sessions);

        void Execute(const SessionsRemoveReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
