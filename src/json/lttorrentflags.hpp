#pragma once

#include <optional>

#include <libtorrent/torrent_flags.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace libtorrent
{
    static void from_json(const json& j, libtorrent::torrent_flags_t& flags)
    {
    }

    static void to_json(json& j, const libtorrent::torrent_flags_t& flags)
    {
    }
}
