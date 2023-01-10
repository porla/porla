#include "torrentcontextprovider.hpp"

#include <libtorrent/torrent_status.hpp>

using porla::Workflows::TorrentContextProvider;

TorrentContextProvider::TorrentContextProvider(const libtorrent::torrent_status &ts)
    : m_ts(std::make_unique<lt::torrent_status>(ts))
{
}

nlohmann::json TorrentContextProvider::Value()
{
    return {
        {"name", m_ts->name}
    };
}
