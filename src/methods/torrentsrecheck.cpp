#include "torrentsrecheck.hpp"

#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_status.hpp>

#include "../sessions.hpp"

using porla::Methods::TorrentsRecheck;
using porla::Methods::TorrentsRecheckReq;
using porla::Methods::TorrentsRecheckRes;

TorrentsRecheck::TorrentsRecheck(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsRecheck::Invoke(const TorrentsRecheckReq &req, WriteCb<TorrentsRecheckRes> cb)
{
    const auto& state = std::find_if(
        m_sessions.All().begin(),
        m_sessions.All().end(),
        [hash = req.info_hash](const auto& state)
        {
            return state.second->torrents.find(hash) != state.second->torrents.end();
        });

    if (state == m_sessions.All().end())
    {
        return cb.Error(-1, "Torrent not found in any session");
    }

    const auto& handle = state->second->torrents.find(req.info_hash);

    if (handle == state->second->torrents.end())
    {
        return cb.Error(-1, "Torrent not found");
    }

    const auto& [ th, _ ] = handle->second;

    state->second->Recheck(th.info_hashes());

    return cb.Ok(TorrentsRecheckRes{});
}
