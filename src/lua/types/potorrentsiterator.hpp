#pragma once

#include <map>
#include <tuple>

#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_status.hpp>

#include "poquery.hpp"

namespace lt = libtorrent;

namespace porla::Lua::Types
{
    class PoTorrentsIterator
    {
    public:
        explicit PoTorrentsIterator(const std::map<lt::info_hash_t, lt::torrent_status>& torrents, std::optional<PoQuery> query)
            : m_torrents(torrents)
            , m_iterator(m_torrents.begin())
            , m_query(query)
        {
        }

        std::optional<std::tuple<lt::torrent_handle, lt::torrent_status>> operator()();

    private:
        std::map<lt::info_hash_t, lt::torrent_status> const&          m_torrents;
        std::map<lt::info_hash_t, lt::torrent_status>::const_iterator m_iterator;
        std::optional<PoQuery>                                        m_query;
    };

}
