#include "torrentsremove.hpp"

#include "../sessions.hpp"

using porla::Methods::TorrentsRemove;
using porla::Methods::TorrentsRemoveReq;
using porla::Methods::TorrentsRemoveRes;

TorrentsRemove::TorrentsRemove(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsRemove::Invoke(const TorrentsRemoveReq &req, WriteCb<TorrentsRemoveRes> cb)
{
    for (const auto& hash : req.info_hashes)
    {
        const auto& state = std::find_if(
            m_sessions.All().begin(),
            m_sessions.All().end(),
            [hash](const auto& state)
            {
                return state.second->torrents.find(hash) != state.second->torrents.end();
            });

        if (state == m_sessions.All().end())
        {
            continue;
        }

        const auto& handle = state->second->torrents.find(hash);

        if (handle == state->second->torrents.end())
        {
            continue;
        }

        const auto& [ th, _ ] = handle->second;

        state->second->session->remove_torrent(
            th,
            req.remove_data ? lt::session::delete_files : lt::remove_flags_t{});
    }

    cb(TorrentsRemoveRes{});
}
