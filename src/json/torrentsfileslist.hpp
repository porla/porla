#pragma once

#include <nlohmann/json.hpp>

#include "../methods/torrentsfileslist_reqres.hpp"

namespace porla::Methods
{
    static void from_json(const nlohmann::json& j, TorrentsFilesListReq& req)
    {
        j.at("info_hash").get_to(req.info_hash);
    }

    static void to_json(nlohmann::json& j, const TorrentsFilesListRes& res)
    {
        json files = json::array();

        for (int i = 0; i < res.file_storage.num_files(); i++)
        {
            lt::file_index_t idx{i};
            auto const& storage = res.file_storage;

            files.push_back({
                {"absolute_path",    storage.file_absolute_path(idx)},
                {"first_block_node", storage.file_first_block_node(idx)},
                {"first_piece_node", storage.file_first_piece_node(idx)},
                {"flags",            static_cast<uint8_t>(storage.file_flags(idx))},
                {"name",             storage.file_name(idx)},
                {"num_blocks",       storage.file_num_blocks(idx)},
                {"num_pieces",       storage.file_num_pieces(idx)},
                {"offset",           storage.file_offset(idx)},
                {"path",             storage.file_path(idx)},
                {"size",             storage.file_size(idx)}
            });
        }

        j = {
            {"files", files}
        };
    }
}
