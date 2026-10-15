#include "torrentaction.hpp"

using porla::Rpc::Methods::Torrents::TorrentAction;

TorrentAction::TorrentAction(porla::Sessions& sessions, Fn fn)
    : m_sessions(sessions)
    , m_fn(std::move(fn))
{
}

void TorrentAction::Execute(const TorrentRef& req, ResponseWriterHandle cb)
{
    const auto resolved = ResolveTorrent(m_sessions, req.session_id, req.info_hash, cb);

    if (!resolved)
    {
        return;
    }

    m_fn(*resolved);

    cb->Ok();
}
