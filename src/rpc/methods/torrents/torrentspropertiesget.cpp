#include "torrentspropertiesget.hpp"

#include "resolve.hpp"

#include "../../../data/models/sessions.hpp"
#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"
#include "../../../sessions/torrent.hpp"

using porla::Rpc::Methods::Torrents::TorrentsPropertiesGet;
using porla::Rpc::Methods::Torrents::TorrentsPropertiesGetReq;
using porla::Rpc::Methods::Torrents::TorrentsPropertiesGetRes;

TorrentsPropertiesGet::TorrentsPropertiesGet(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsPropertiesGet::Execute(const TorrentsPropertiesGetReq& req, ResponseWriterHandle cb)
{
    const auto resolved = ResolveTorrent(m_sessions, req.session_id, req.info_hash, cb);

    if (!resolved)
    {
        return;
    }

    cb->Ok(TorrentsPropertiesGetRes{
        .download_limit  = resolved->torrent->status.handle.download_limit(),
        .flags           = resolved->torrent->status.handle.flags(),
        .max_connections = resolved->torrent->status.handle.max_connections(),
        .max_uploads     = resolved->torrent->status.handle.max_uploads(),
        .upload_limit    = resolved->torrent->status.handle.upload_limit()
    });
}
