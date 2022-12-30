#pragma once

#include <memory>

#include <libtorrent/fwd.hpp>

#include "contextprovider.hpp"

namespace porla::Workflows
{
    class TorrentContextProvider : public ContextProvider
    {
    public:
        explicit TorrentContextProvider(const libtorrent::torrent_status& ts);

        nlohmann::json ResolveSegments(const std::vector<std::string>& segments) override;

    private:
        std::unique_ptr<libtorrent::torrent_status> m_ts;
    };
}
