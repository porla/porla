#include "torrentsfileslist.hpp"

#include "../sessions.hpp"

using porla::Methods::TorrentsFilesList;
using porla::Methods::TorrentsFilesListReq;
using porla::Methods::TorrentsFilesListRes;

TorrentsFilesList::TorrentsFilesList(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsFilesList::Invoke(const TorrentsFilesListReq& req, WriteCb<TorrentsFilesListRes> cb)
{
    auto const& torrents = m_sessions.Default()->torrents;
    auto const handle = torrents.find(req.info_hash);

    if (handle == torrents.end())
    {
        return cb.Error(-1, "Torrent not found");
    }

    auto const status = handle->second.status();

    TorrentsFilesListRes res;

    if (auto tf = status.torrent_file.lock())
    {
        return cb.Ok(TorrentsFilesListRes{
            .file_storage = tf->files()
        });
    }

    return cb.Error(-2, "Failed to lock torrent file");
}
