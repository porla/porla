#include "torrentspause.hpp"

#include "../sessions.hpp"

using porla::Methods::TorrentsPause;
using porla::Methods::TorrentsPauseReq;
using porla::Methods::TorrentsPauseRes;

template <bool SSL> TorrentsPause<SSL>::TorrentsPause(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

template <bool SSL> void TorrentsPause<SSL>::Invoke(const TorrentsPauseReq& req, WriteCb<TorrentsPauseRes, SSL> cb)
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

    th.pause();

    cb.Ok(TorrentsPauseRes{});
}

namespace porla::Methods {
    template class TorrentsPause<true>;
    template class TorrentsPause<false>;
}
