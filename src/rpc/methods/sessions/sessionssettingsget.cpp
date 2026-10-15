#include "sessionssettingsget.hpp"

#include "../resolve.hpp"

#include "../../../data/models/sessions.hpp"
#include "../../../json/lttypes.hpp"
#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"

using porla::Rpc::Methods::Sessions::SessionsSettingsGet;

SessionsSettingsGet::SessionsSettingsGet(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void SessionsSettingsGet::Execute(const SessionRef &req, ResponseWriterHandle cb)
{
    const auto session = FindSession(m_sessions, req.id, cb);

    if (!session)
    {
        return;
    }

    const auto& state = m_sessions.Get(session->id);

    cb->Ok({
        {"settings", state == nullptr
            ? session->params.settings
            : state->Libtorrent().get_settings()}
    });
}
