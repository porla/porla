#pragma once

#include <libtorrent/torrent_handle.hpp>
#include <sol/sol.hpp>

namespace porla
{
    class TorrentClientData;
}

namespace porla::Lua::Types
{
    struct PoTorrentData
    {
        static void Register(sol::state& lua);

        explicit PoTorrentData(lt::torrent_handle th);

    private:
        TorrentClientData& ClientData() const;

        lt::torrent_handle m_th;
    };
}
