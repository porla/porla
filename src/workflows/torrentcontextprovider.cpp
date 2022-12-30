#include "torrentcontextprovider.hpp"

#include <libtorrent/torrent_status.hpp>

using porla::Workflows::TorrentContextProvider;

TorrentContextProvider::TorrentContextProvider(const libtorrent::torrent_status &ts)
    : m_ts(std::make_unique<lt::torrent_status>(ts))
{
}

nlohmann::json TorrentContextProvider::ResolveSegments(const std::vector<std::string> &segments)
{
    if (segments.size() == 1 && segments.at(0) == "name")
    {
        return m_ts->name;
    }

    return {};
}
