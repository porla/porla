#include "torrentsfilesprioritize.hpp"

#include "resolve.hpp"

#include "../../../data/models/sessions.hpp"
#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"
#include "../../../sessions/torrent.hpp"

using porla::Rpc::Methods::Torrents::TorrentsFilesPrioritize;
using porla::Rpc::Methods::Torrents::TorrentsFilesPrioritizeReq;
using porla::Rpc::Methods::Torrents::TorrentsFilesPrioritizeRes;

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

    std::vector<lt::download_priority_t> file_prios = resolved->torrent->status.handle.get_file_priorities();

    for (const auto& fp : req.priorities)
    {
        const auto index = static_cast<int>(fp.index);

        if (index < 0 || index >= file_prios.size())
        {
            continue;
        }

        file_prios[index] = fp.priority;
    }

    resolved->torrent->status.handle.prioritize_files(file_prios);

    cb->Ok({});
}
