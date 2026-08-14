#include "../all.hpp"

#include <unordered_set>

#include "../../rpc/methods/torrents/torrentsfileslist_reqres.hpp"
#include "../utils.hpp"

namespace libtorrent
{
    void to_json(nlohmann::json& j, const libtorrent::file_flags_t& flags)
    {
        std::unordered_set<std::string> f;
        if (flags & lt::file_storage::flag_pad_file)   f.insert("pad_file");
        if (flags & lt::file_storage::flag_hidden)     f.insert("hidden");
        if (flags & lt::file_storage::flag_executable) f.insert("executable");
        if (flags & lt::file_storage::flag_symlink)    f.insert("symlink");

        j = f;
    }
}

namespace porla::Rpc::Methods::Torrents
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsFilesListReq,
        info_hash,
        session_id)

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
}
