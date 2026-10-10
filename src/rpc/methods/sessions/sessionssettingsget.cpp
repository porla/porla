#include "sessionssettingsget.hpp"

#include "../../../data/models/sessions.hpp"
#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"

using porla::Rpc::Methods::Sessions::SessionsSettingsGet;
using porla::Rpc::Methods::Sessions::SessionsSettingsGetReq;
using porla::Rpc::Methods::Sessions::SessionsSettingsGetRes;

SessionsSettingsGet::SessionsSettingsGet(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void SessionsSettingsGet::Execute(const SessionsSettingsGetReq &req, ResponseWriterHandle cb)
{
    const auto session = m_sessions.Find(req.id);

    if (!session)
    {
        return cb->Error(-1, "Session not found");
    }

    const auto& state = m_sessions.Get(session->id);

    cb->Ok(SessionsSettingsGetRes{
        .settings = state == nullptr
            ? session->params.settings
            : state->Libtorrent().get_settings()
    });
}
