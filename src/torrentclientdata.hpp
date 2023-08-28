#pragma once

#include <map>
#include <optional>
#include <string>
#include <unordered_set>

#include <nlohmann/json.hpp>

#include "sessions.hpp"

namespace porla
{

    struct TorrentClientData
    {
        std::optional<bool>                                  ignore_alert;
        std::optional<std::string>                           category;
        std::optional<std::map<std::string, nlohmann::json>> metadata;
        std::unordered_set<std::string>                      tags;

        std::weak_ptr<Sessions::SessionState> state;
    };
}
