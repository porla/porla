#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"
#include "sessionssettingsset_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Sessions
{
    class SessionsSettingsSet : public TypedMethod<SessionsSettingsSetReq, SessionsSettingsSetRes>
    {
    public:
        explicit SessionsSettingsSet(sqlite3* db, porla::Sessions& sessions);

    protected:
        void Execute(const SessionsSettingsSetReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
