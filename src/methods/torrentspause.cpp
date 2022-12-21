#include "torrentspause.hpp"

#include "../session.hpp"

using porla::Methods::TorrentsPause;
using porla::Methods::TorrentsPauseReq;
using porla::Methods::TorrentsPauseRes;

TorrentsPause::TorrentsPause(porla::ISession &session)
    : m_session(session)
{
}

void TorrentsPause::Invoke(const TorrentsPauseReq& req, WriteCb<TorrentsPauseRes> cb)
{
    auto const& torrents = m_session.Torrents();
    auto handle = torrents.find(req.info_hash);

    if (handle == torrents.end())
    {
        return cb.Error(-1, "Torrent not found");
    }

    handle->second.pause();

    cb.Ok(TorrentsPauseRes{});
}
