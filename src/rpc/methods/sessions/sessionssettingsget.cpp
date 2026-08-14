#include "sessionssettingsget.hpp"

#include "../../../data/models/sessions.hpp"
#include "../../../sessions.hpp"

using porla::Rpc::Methods::Sessions::SessionsSettingsGet;
using porla::Rpc::Methods::Sessions::SessionsSettingsGetReq;
using porla::Rpc::Methods::Sessions::SessionsSettingsGetRes;

SessionsSettingsGet::SessionsSettingsGet(sqlite3* db, porla::Sessions& sessions)
    : m_db(db)
    , m_sessions(sessions)
{
}

void SessionsSettingsGet::Execute(const SessionsSettingsGetReq &req, ResponseWriterHandle cb)
{
    const auto session = Data::Models::Sessions::GetById(m_db, req.id);

    if (!session)
    {
        return cb->Error(-1, "Session not found");
    }

    const auto& state = m_sessions.Get(session->id);

    cb->Ok(SessionsSettingsGetRes{
        .settings = state == nullptr
            ? session->params.settings
            : state->session->get_settings()
    });
}
