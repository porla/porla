#include "torrentsremove.hpp"

#include "resolve.hpp"

#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"
#include "../../../sessions/torrent.hpp"

using porla::Rpc::Methods::Torrents::TorrentsRemove;
using porla::Rpc::Methods::Torrents::TorrentsRemoveReq;
using porla::Rpc::Methods::Torrents::TorrentsRemoveRes;

TorrentsRemove::TorrentsRemove(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsRemove::Execute(const TorrentsRemoveReq &req, ResponseWriterHandle cb)
{
    const auto resolved = ResolveTorrent(m_sessions, req.session_id, req.info_hash, cb);

    if (!resolved)
    {
        return;
    }

    resolved->session->Libtorrent().remove_torrent(
        resolved->torrent->status.handle,
        req.remove_data.value_or(false)
            ? lt::session::delete_files
            : lt::remove_flags_t{});

    cb->Ok(TorrentsRemoveRes{});
}
