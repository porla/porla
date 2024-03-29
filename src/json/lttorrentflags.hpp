#pragma once

#include <optional>

#include <libtorrent/torrent_flags.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace libtorrent
{
    static void from_json(const json& j, libtorrent::torrent_flags_t& flags)
    {
        flags = static_cast<libtorrent::torrent_flags_t>(j.get<uint64_t>());
    }

    static void to_json(json& j, const libtorrent::torrent_flags_t& flags)
    {
        j = static_cast<uint64_t>(flags);
    }
}
