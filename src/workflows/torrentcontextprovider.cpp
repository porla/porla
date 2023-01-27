#include "torrentcontextprovider.hpp"

#include <libtorrent/torrent_status.hpp>

#include "../json/lttorrentstatus.hpp"
#include "../json/mediainfo.hpp"
#include "../torrentclientdata.hpp"

using porla::Workflows::TorrentContextProvider;

TorrentContextProvider::TorrentContextProvider(const libtorrent::torrent_status &ts)
    : m_ts(std::make_unique<lt::torrent_status>(ts))
{
}

nlohmann::json TorrentContextProvider::Value()
{
    nlohmann::json torrent = *m_ts;
    torrent["mediainfo"] = json::object();

    if (const auto client_data = m_ts->handle.userdata().get<TorrentClientData>())
    {
        if (const auto mediainfo = client_data->mediainfo)
        {
            torrent["mediainfo"] = *mediainfo;
        }
    }

    return torrent;
}
