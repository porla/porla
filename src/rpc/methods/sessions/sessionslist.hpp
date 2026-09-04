#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"
#include "sessionslist_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Sessions
{
    class SessionsList : public TypedMethod<SessionsListReq, SessionsListRes>
    {
    public:
        explicit SessionsList(sqlite3* db, porla::Sessions& sessions);

        void Execute(const SessionsListReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
