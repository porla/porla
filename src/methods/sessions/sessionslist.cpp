#include "sessionslist.hpp"

#include "../../sessions.hpp"

using porla::Methods::SessionsList;
using porla::Methods::SessionsListReq;
using porla::Methods::SessionsListRes;

SessionsList::SessionsList(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void SessionsList::Invoke(const SessionsListReq& req, WriteCb<SessionsListRes> cb)
{
    std::vector<SessionsListRes::Item> session_items;

    for (const auto& [ _, state ] : m_sessions.All())
    {
        session_items.push_back(SessionsListRes::Item{
            .id             = state->id,
            .name           = state->name,
            .metadata       = state->metadata,
            .torrents_total = static_cast<int>(state->torrents.size())
        });
    }

    cb.Ok(SessionsListRes{
        .sessions = session_items
    });
}
