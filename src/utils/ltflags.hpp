#pragma once

#include <map>
#include <optional>
#include <string>

#include <libtorrent/torrent_flags.hpp>

namespace porla::Utils
{
    class LtFlags
    {
    public:
        static const std::map<std::string, lt::torrent_flags_t>& AllTorrentFlags();
        static std::optional<lt::torrent_flags_t> FindTorrentFlag(const std::string& name);
    };
}
