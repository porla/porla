#include "torrentsfileslist.hpp"

#include "../session.hpp"

using porla::Methods::TorrentsFilesList;
using porla::Methods::TorrentsFilesListReq;
using porla::Methods::TorrentsFilesListRes;

TorrentsFilesList::TorrentsFilesList(porla::ISession &session)
    : m_session(session)
{
}

void TorrentsFilesList::Invoke(const TorrentsFilesListReq& req, WriteCb<TorrentsFilesListRes> cb)
{
    auto const& torrents = m_session.Torrents();
    auto const status = torrents.find(req.info_hash);

    if (status == torrents.end())
    {
        return cb.Error(-1, "Torrent not found");
    }

    TorrentsFilesListRes res;

    if (auto tf = status->second.torrent_file.lock())
    {
        return cb.Ok(TorrentsFilesListRes{
            .file_storage = tf->files()
        });
    }

    return cb.Error(-2, "Failed to lock torrent file");
}
