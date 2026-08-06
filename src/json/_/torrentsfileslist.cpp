#include "../all.hpp"

#include <unordered_set>

#include "../../methods/torrents/torrentsfileslist_reqres.hpp"
#include "../utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsFilesListReq,
        info_hash,
        session_id)

    void to_json(nlohmann::json& json, const TorrentsFilesListRes& res)
    {
        nlohmann::json files = nlohmann::json::array();

        const auto& storage = res.file_storage;

        for (int i = 0; i < storage.num_files(); i++)
        {
            lt::file_index_t idx{i};

            const auto lt_flags = storage.file_flags(idx);

            std::unordered_set<std::string> flags;
            if (lt_flags & lt::file_storage::flag_pad_file)   flags.insert("pad_file");
            if (lt_flags & lt::file_storage::flag_hidden)     flags.insert("hidden");
            if (lt_flags & lt::file_storage::flag_executable) flags.insert("executable");
            if (lt_flags & lt::file_storage::flag_symlink)    flags.insert("symlink");

            files.push_back({
                {"absolute_path",    storage.file_absolute_path(idx)},
                {"first_block_node", storage.file_first_block_node(idx)},
                {"first_piece_node", storage.file_first_piece_node(idx)},
                {"flags",            flags},
                {"name",             storage.file_name(idx)},
                {"num_blocks",       storage.file_num_blocks(idx)},
                {"num_pieces",       storage.file_num_pieces(idx)},
                {"offset",           storage.file_offset(idx)},
                {"path",             storage.file_path(idx)},
                {"size",             storage.file_size(idx)},
                {"symlink",          storage.symlink(idx)}
            });
        }

        json = {
            {"files", files}
        };
    }
}
