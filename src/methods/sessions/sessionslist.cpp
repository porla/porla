#include "sessionslist.hpp"

#include "../../sessions.hpp"

using porla::Methods::SessionsList;
using porla::Methods::SessionsListReq;
using porla::Methods::SessionsListRes;

template <bool SSL> SessionsList<SSL>::SessionsList(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

template <bool SSL> void SessionsList<SSL>::Invoke(const SessionsListReq& req, WriteCb<SessionsListRes, SSL> cb)
{
    std::vector<SessionsListRes::Item> session_items;

    for (const auto& [ name, state ] : m_sessions.All())
    {
        session_items.push_back(SessionsListRes::Item{
            .name           = name,
            .is_dht_running = state->session->is_dht_running(),
            .is_listening   = state->session->is_listening(),
            .is_paused      = state->session->is_paused(),
            .torrents_total = static_cast<int>(state->torrents.size())
        });
    }

    cb.Ok(SessionsListRes{
        .sessions = session_items
    });
}

namespace porla::Methods {
    template class SessionsList<true>;
    template class SessionsList<false>;
}
