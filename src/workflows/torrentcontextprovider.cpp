#include "torrentcontextprovider.hpp"

#include <libtorrent/torrent_status.hpp>

#include "../json/lttorrentstatus.hpp"
#include "../torrentclientdata.hpp"

using porla::Workflows::TorrentContextProvider;

TorrentContextProvider::TorrentContextProvider(const libtorrent::torrent_status &ts)
    : m_ts(std::make_unique<lt::torrent_status>(ts))
{
}

nlohmann::json TorrentContextProvider::Value()
{
    nlohmann::json torrent = *m_ts;

    if (const auto client_data = m_ts->handle.userdata().get<TorrentClientData>())
    {
    }

    return torrent;
}
