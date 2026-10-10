#include "torrentsfilespriorities.hpp"

#include "resolve.hpp"

#include "../../../data/models/sessions.hpp"
#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"
#include "../../../sessions/torrent.hpp"

using porla::Rpc::Methods::Torrents::TorrentsFilesPriorities;
using porla::Rpc::Methods::Torrents::TorrentsFilesPrioritiesReq;
using porla::Rpc::Methods::Torrents::TorrentsFilesPrioritiesRes;

TorrentsFilesPriorities::TorrentsFilesPriorities(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsFilesPriorities::Execute(const TorrentsFilesPrioritiesReq& req, ResponseWriterHandle cb)
{
    const auto resolved = ResolveTorrent(m_sessions, req.session_id, req.info_hash, cb);

    if (!resolved)
    {
        return;
    }

    cb->Ok(TorrentsFilesPrioritiesRes{
        .priorities = resolved->torrent->status.handle.get_file_priorities()
    });
}
