#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

#include <libtorrent/peer_info.hpp>
#include <libtorrent/torrent_flags.hpp>

namespace porla::Utils
{
    class LtFlags
    {
    public:
        static const std::map<std::string, lt::torrent_flags_t>& AllTorrentFlags();
        static std::optional<lt::torrent_flags_t> FindTorrentFlag(const std::string& name);

        static std::string              ConnectionTypeName(lt::connection_type_t type);
        static std::vector<std::string> PeerFlagNames(lt::peer_flags_t flags);
        static std::vector<std::string> PeerSourceNames(lt::peer_source_flags_t source);
    };
}
