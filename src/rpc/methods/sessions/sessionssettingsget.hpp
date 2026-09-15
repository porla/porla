#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"
#include "sessionssettingsget_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Sessions
{
    class SessionsSettingsGet : public TypedMethod<SessionsSettingsGetReq, SessionsSettingsGetRes>
    {
    public:
        explicit SessionsSettingsGet(sqlite3* db, porla::Sessions& sessions);

    protected:
        void Execute(const SessionsSettingsGetReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
