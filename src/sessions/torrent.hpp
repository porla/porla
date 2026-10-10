#pragma once

#include <memory>
#include <optional>

#include <libtorrent/torrent_status.hpp>

namespace porla
{
    struct TorrentClientData;

    struct Torrent
    {
        enum class State
        {
            // passed to async_add_torrent from storage
            Loading,

            // passed to async_add_torrent from user
            Adding,

            // torrent is in libtorrent, visible to all
            Current
        };

        State                              state = State::Adding;
        lt::torrent_status                 status;
        std::unique_ptr<TorrentClientData> data;

        // for torrents that lets libtorrent fetch metadata, track whether we
        // received metadata and whether we saved metadata
        bool metadata_announced = false;
        bool metadata_saved     = false;

        // when we do a recheck of a torrent, store its flags
        // so we can restore them afterwards
        std::optional<lt::torrent_flags_t> restore_after_check;
    };
}
