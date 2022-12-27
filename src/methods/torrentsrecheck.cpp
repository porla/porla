#include "torrentsrecheck.hpp"

#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_status.hpp>

#include "../session.hpp"

using porla::Methods::TorrentsRecheck;
using porla::Methods::TorrentsRecheckReq;
using porla::Methods::TorrentsRecheckRes;

TorrentsRecheck::TorrentsRecheck(porla::ISession &session)
    : m_session(session)
{
}

void TorrentsRecheck::Invoke(const TorrentsRecheckReq &req, WriteCb<TorrentsRecheckRes> cb)
{
    auto const& torrents = m_session.Torrents();
    auto const& handle = torrents.find(req.info_hash);

    if (handle == torrents.end())
    {
        return cb.Error(-1, "Torrent not found");
    }

    m_session.Recheck(req.info_hash);

    return cb.Ok(TorrentsRecheckRes{});
}
