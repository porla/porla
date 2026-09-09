#pragma once

#include <map>

#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_status.hpp>

namespace lt = libtorrent;

namespace porla::Lua::Types
{
    class PoTorrentsIterator
    {
    public:
        explicit PoTorrentsIterator(const std::map<lt::info_hash_t, std::tuple<lt::torrent_handle, lt::torrent_status>>& torrents)
            : m_torrents(torrents)
            , m_iterator(m_torrents.begin())
        {
        }

        std::optional<lt::torrent_handle> operator()();

    private:
        std::map<lt::info_hash_t, std::tuple<lt::torrent_handle, lt::torrent_status>> const&          m_torrents;
        std::map<lt::info_hash_t, std::tuple<lt::torrent_handle, lt::torrent_status>>::const_iterator m_iterator;
    };

}
