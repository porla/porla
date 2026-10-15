#include "sessionsget.hpp"

#include "../resolve.hpp"
#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"

using porla::Rpc::Methods::Sessions::SessionsGet;
using porla::Rpc::Methods::Sessions::SessionsGetRes;

SessionsGet::SessionsGet(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void SessionsGet::Execute(const SessionRef &req, ResponseWriterHandle cb)
{
    const auto session = FindSession(m_sessions, req.id, cb);

    if (!session)
    {
        return;
    }

    const auto& state = m_sessions.Get(req.id);

    cb->Ok(SessionsGetRes{
        .session = SessionsGetRes::Session{
            .id         = session->id,
            .name       = session->name,
            .is_default = session->is_default,
            .metadata   = session->metadata,
            .state      = state == nullptr
                ? std::optional<SessionsGetRes::SessionState>()
                : SessionsGetRes::SessionState{
                    .is_listening   = state->Libtorrent().is_listening(),
                    .is_paused      = state->Libtorrent().is_paused(),
                    .torrents_total = static_cast<int>(state->Count())
                }
        }
    });
}
