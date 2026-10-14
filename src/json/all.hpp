#pragma once

#include <nlohmann/json.hpp>

#include "lttypes.hpp"

namespace boost::system
{
    class error_code;

    inline void from_json(const nlohmann::json& json, error_code& ec) {}
    void to_json(nlohmann::json& json, const error_code& ec);
}

namespace porla
{
    class TorrentClientData;
    void to_json(nlohmann::json& json, const TorrentClientData& tcd);
}

namespace porla::Rpc::Methods
{
    namespace Torrents
    {
        struct TorrentsFilesListReq;
        struct TorrentsFilesListRes;

        void from_json(const nlohmann::json& json, TorrentsFilesListReq& req);
        void to_json(nlohmann::json& json, const TorrentsFilesListRes& res);
    }
}
