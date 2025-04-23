#include "torrentsrecheck.hpp"

#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_status.hpp>

#include "../sessions.hpp"

using porla::Methods::TorrentsRecheck;
using porla::Methods::TorrentsRecheckReq;
using porla::Methods::TorrentsRecheckRes;

template <bool SSL> TorrentsRecheck<SSL>::TorrentsRecheck(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

template <bool SSL> void TorrentsRecheck<SSL>::Invoke(const TorrentsRecheckReq &req, WriteCb<TorrentsRecheckRes, SSL> cb)
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

    if (!th.is_valid())
    {
        return cb.Error(-2, "Torrent not valid");
    }

    state->second->Recheck(th.info_hashes());

    return cb.Ok(TorrentsRecheckRes{});
}

namespace porla::Methods {
    template class TorrentsRecheck<true>;
    template class TorrentsRecheck<false>;
}
