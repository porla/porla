#include "torrentsfileslist.hpp"

#include <algorithm>

#include "resolve.hpp"

#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"
#include "../../../sessions/torrent.hpp"

using porla::Rpc::Methods::Torrents::TorrentsFilesList;
using porla::Rpc::Methods::Torrents::TorrentsFilesListReq;
using porla::Rpc::Methods::Torrents::TorrentsFilesListRes;

TorrentsFilesList::TorrentsFilesList(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsFilesList::Execute(const TorrentsFilesListReq& req, ResponseWriterHandle cb)
{
    const auto resolved = ResolveTorrent(
        m_sessions,
        req.session_id,
        req.info_hash,
        cb);

    if (!resolved)
    {
        return;
    }

    if (auto tf = resolved->torrent->status.torrent_file.lock())
    {
        return cb->Ok(TorrentsFilesListRes{
            .file_storage  = tf->layout(),
            .renamed_files = resolved->torrent->status.handle.get_renamed_files()
        });
    }

    return cb->Error(-4, "Failed to lock torrent file");
}
