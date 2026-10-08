#pragma once

#include <functional>

#include <libtorrent/torrent_handle.hpp>
#include <sol/sol.hpp>

namespace porla
{
    struct TorrentClientData;
    class Torrents;
}

namespace porla::Lua::Types
{
    struct PoTorrentData
    {
        static void Register(sol::state& lua);

        explicit PoTorrentData(lt::torrent_handle th, porla::Torrents& torrents);

    private:
        TorrentClientData& ClientData() const;
        void Update(const std::function<void(TorrentClientData&)>& change) const;

        lt::torrent_handle m_th;
        porla::Torrents&   m_torrents;
    };
}
