#pragma once

#include "sessionevent.hpp"

#include <libtorrent/info_hash.hpp>
#include <libtorrent/torrent_handle.hpp>

namespace porla
{
    struct TorrentEvent : SessionEvent
    {
        using SessionEvent::SessionEvent;

        lt::torrent_handle torrent_handle;
        lt::info_hash_t    info_hash;
    };
}
