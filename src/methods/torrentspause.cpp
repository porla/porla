#include "torrentspause.hpp"

#include "../sessions.hpp"

using porla::Methods::TorrentsPause;
using porla::Methods::TorrentsPauseReq;
using porla::Methods::TorrentsPauseRes;

TorrentsPause::TorrentsPause(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsPause::Invoke(const TorrentsPauseReq& req, WriteCb<TorrentsPauseRes> cb)
{
    auto const& torrents = m_sessions.Default()->torrents;
    auto handle = torrents.find(req.info_hash);

    if (handle == torrents.end())
    {
        return cb.Error(-1, "Torrent not found");
    }

    handle->second.pause();

    cb.Ok(TorrentsPauseRes{});
}
