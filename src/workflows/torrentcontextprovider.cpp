#include "torrentcontextprovider.hpp"

#include <libtorrent/torrent_status.hpp>

#include "../json/lttorrentstatus.hpp"

using porla::Workflows::TorrentContextProvider;

TorrentContextProvider::TorrentContextProvider(const libtorrent::torrent_status &ts)
    : m_ts(std::make_unique<lt::torrent_status>(ts))
{
}

nlohmann::json TorrentContextProvider::Value()
{
    return *m_ts;
}
