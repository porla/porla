#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"
#include "sessionspause_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Sessions
{
    class SessionsPause : public TypedMethod<SessionsPauseReq, SessionsPauseRes>
    {
    public:
        explicit SessionsPause(sqlite3* db, porla::Sessions& sessions);

    protected:
        void Execute(const SessionsPauseReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
