#include "torrentsmediainfo.hpp"

#include "../session.hpp"
#include "../torrentclientdata.hpp"

using porla::Methods::TorrentsMediaInfo;
using porla::Methods::TorrentsMediaInfoReq;
using porla::Methods::TorrentsMediaInfoRes;

TorrentsMediaInfo::TorrentsMediaInfo(porla::ISession &session)
    : m_session(session)
{
}

void TorrentsMediaInfo::Invoke(const TorrentsMediaInfoReq &req, WriteCb<TorrentsMediaInfoRes> cb)
{
    auto const& torrents = m_session.Torrents();
    auto const& handle = torrents.find(req.info_hash);

    if (handle == torrents.end())
    {
        return cb.Error(-1, "Torrent not found");
    }

    return cb.Ok(TorrentsMediaInfoRes{
        .mediainfo = handle->second.userdata().get<TorrentClientData>()->mediainfo
    });
}
