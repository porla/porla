#pragma once

#include <algorithm>
#include <chrono>
#include <memory>
#include <optional>

#include <libtorrent/time.hpp>
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

        // when status was stored (or last caught up). unset until the first status
        // from libtorrent, e.g. while a torrent loaded from the db waits for it
        lt::time_point time_refreshed_at;

        // for torrents that lets libtorrent fetch metadata, track whether we
        // received metadata and whether we saved metadata
        bool metadata_announced = false;
        bool metadata_saved     = false;

        // when we do a recheck of a torrent, store its flags
        // so we can restore them afterwards
        std::optional<lt::torrent_flags_t> restore_after_check;

        // libtorrent only posts a status when it changes, so the fields it moves with
        // the clock stand still while a torrent is idle. this brings them up to now,
        // in whole seconds like libtorrent counts them. every fresh status replaces
        // the extrapolation, so errors don't add up.
        void RefreshTimestamps(lt::time_point now, bool session_paused)
        {
            if (time_refreshed_at == lt::time_point{})
            {
                return;
            }

            const auto elapsed = std::chrono::floor<std::chrono::seconds>(now - time_refreshed_at);

            if (elapsed <= std::chrono::seconds::zero())
            {
                return;
            }

            // libtorrent runs its timers while neither the torrent nor its session is paused
            const bool live = !(status.flags & lt::torrent_flags::paused) && !session_paused;

            if (live)                       status.active_duration   += elapsed;
            if (live && status.is_seeding)  status.seeding_duration  += elapsed;
            if (live && status.is_finished) status.finished_duration += elapsed;

            status.next_announce = std::max(lt::time_duration::zero(), status.next_announce - elapsed);

            // move by the whole seconds added, so the remainder counts next time
            time_refreshed_at += elapsed;
        }
    };
}
