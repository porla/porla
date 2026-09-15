#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"
#include "sessionsget_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Sessions
{
    class SessionsGet : public TypedMethod<SessionsGetReq, SessionsGetRes>
    {
    public:
        explicit SessionsGet(sqlite3* db, porla::Sessions& sessions);

    protected:
        void Execute(const SessionsGetReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
