#pragma once

#include <optional>
#include <string>
#include <unordered_set>

namespace porla
{
    struct TorrentClientData
    {
        std::optional<std::string>      category;
        std::unordered_set<std::string> tags;
    };
}
