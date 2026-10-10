#include "torrentsfilesrename.hpp"

#include "resolve.hpp"

#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"
#include "../../../sessions/torrent.hpp"

using porla::Rpc::Methods::Torrents::TorrentsFilesRename;
using porla::Rpc::Methods::Torrents::TorrentsFilesRenameReq;
using porla::Rpc::Methods::Torrents::TorrentsFilesRenameRes;

TorrentsFilesRename::TorrentsFilesRename(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsFilesRename::Execute(const TorrentsFilesRenameReq& req, ResponseWriterHandle cb)
{
    const auto resolved = ResolveTorrent(m_sessions, req.session_id, req.info_hash, cb);

    if (!resolved)
    {
        return;
    }

    resolved->torrent->status.handle.rename_file(
        lt::file_index_t{req.file_index},
        req.file_path);

    cb->Ok({});
}
