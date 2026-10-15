#include "sessionsupdate.hpp"

#include <boost/log/trivial.hpp>

#include "../resolve.hpp"

#include "../../../data/models/sessions.hpp"
#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"

using porla::Rpc::Methods::Sessions::SessionsUpdate;
using porla::Rpc::Methods::Sessions::SessionsUpdateReq;

SessionsUpdate::SessionsUpdate(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void SessionsUpdate::Execute(const SessionsUpdateReq& req, ResponseWriterHandle cb)
{
    auto session = FindSession(m_sessions, req.id, cb);

    if (!session)
    {
        return;
    }

    if (req.timer_dht_stats && *req.timer_dht_stats < 10)
    {
        return cb->Error(-32602, "Too narrow timer interval", {{"field","timer_dht_stats"}});
    }

    if (req.timer_save_state && *req.timer_save_state < 10)
    {
        return cb->Error(-32602, "Too narrow timer interval", {{"field","timer_save_state"}});
    }

    if (req.timer_session_stats && *req.timer_session_stats < 10)
    {
        return cb->Error(-32602, "Too narrow timer interval", {{"field","timer_session_stats"}});
    }

    if (req.timer_torrent_updates && *req.timer_torrent_updates < 10)
    {
        return cb->Error(-32602, "Too narrow timer interval", {{"field","timer_torrent_updates"}});
    }

    session->name                  = req.name;
    session->is_default            = req.is_default;
    session->metadata              = req.metadata;
    session->timer_dht_stats       = req.timer_dht_stats.value_or(session->timer_dht_stats);
    session->timer_save_state      = req.timer_save_state.value_or(session->timer_save_state);
    session->timer_session_stats   = req.timer_session_stats.value_or(session->timer_session_stats);
    session->timer_torrent_updates = req.timer_torrent_updates.value_or(session->timer_torrent_updates);

    m_sessions.Update(*session);

    cb->Ok();
}
