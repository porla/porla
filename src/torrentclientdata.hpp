#pragma once

#include <map>
#include <optional>
#include <string>
#include <unordered_set>

#include <nlohmann/json.hpp>

namespace porla
{
    class Session;

    struct TorrentClientData
    {
        std::optional<std::string>            category     = std::nullopt;
        std::optional<std::int64_t>           completed_at = std::nullopt;
        std::map<std::string, nlohmann::json> metadata     = {};
        std::unordered_set<std::string>       tags         = {};

        std::weak_ptr<Session>                session      = {};
    };
}
