#pragma once

#include <nlohmann/json.hpp>

#include "../methods/torrentsfileslist_reqres.hpp"

namespace porla::Methods
{
    static void from_json(const nlohmann::json& j, TorrentsFilesListReq& req)
    {
    }

    static void to_json(nlohmann::json& j, const TorrentsFilesListRes::FileItem& fi)
    {
    }

    static void to_json(nlohmann::json& j, const TorrentsFilesListRes& res)
    {
    }
}
