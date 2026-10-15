#include "torrentsfileslist.hpp"

#include <algorithm>

#include "../resolve.hpp"

#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"
#include "../../../sessions/torrent.hpp"

struct TorrentsFilesListRes
{
    libtorrent::file_storage  file_storage;
    libtorrent::renamed_files renamed_files;
};

void to_json(nlohmann::json& json, const TorrentsFilesListRes& res);

using porla::Rpc::Methods::Torrents::TorrentsFilesList;

TorrentsFilesList::TorrentsFilesList(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsFilesList::Execute(const TorrentRef& req, ResponseWriterHandle cb)
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

void to_json(nlohmann::json& json, const TorrentsFilesListRes& res)
{
    nlohmann::json files = nlohmann::json::array();

    const auto& storage = res.file_storage;

    lt::filenames fn(res.file_storage, res.renamed_files);

    for (const auto idx : fn.file_range())
    {
        files.push_back({
            {"absolute_path", fn.file_absolute_path(idx)},
            {"flags",         fn.file_flags(idx)},
            {"index",         static_cast<int>(idx)},
            {"offset",        fn.file_offset(idx)},
            {"path",          fn.file_path(idx)},
            {"size",          fn.file_size(idx)},
            {"symlink",       fn.symlink(idx)}
        });
    }

    json = {
        {"files", files},
    };
}
