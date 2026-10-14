#include "torrentsfilesprioritize.hpp"

#include "resolve.hpp"

#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"
#include "../../../sessions/torrent.hpp"

using porla::Rpc::Methods::Torrents::TorrentsFilesPrioritize;
using porla::Rpc::Methods::Torrents::TorrentsFilesPrioritizeReq;

TorrentsFilesPrioritize::TorrentsFilesPrioritize(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsFilesPrioritize::Execute(const TorrentsFilesPrioritizeReq& req, ResponseWriterHandle cb)
{
    const auto resolved = ResolveTorrent(m_sessions, req.session_id, req.info_hash, cb);

    if (!resolved)
    {
        return;
    }

    const auto tf = resolved->torrent->status.torrent_file.lock();

    if (!tf)
    {
        return cb->Ok({});
    }

    const auto& th        = resolved->torrent->status.handle;
    const auto  num_files = tf->num_files();

    // set each priority on its own - libtorrent merges these while a disk job is running.
    // a get_file_priorities/prioritize_files round trip would block the io thread and
    // could overwrite priorities that libtorrent has queued but not applied yet.
    for (const auto& fp : req.priorities)
    {
        if (fp.index < lt::file_index_t{0} || static_cast<int>(fp.index) >= num_files)
        {
            continue;
        }

        th.file_priority(fp.index, fp.priority);
    }

    cb->Ok();
}
