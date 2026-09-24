#pragma once

#include <libtorrent/torrent_status.hpp>

namespace porla::Utils
{
    // never report ratios above this
    constexpr double MAX_RATIO = 9999;

    // How much a torrent has given back compared to what it cost us. Falls back
    // to the amount of data we hold when the download counter is missing or
    // implausibly small, which is the case for torrents added in seed mode or
    // cross seeded from a download made elsewhere.
    double Ratio(const libtorrent::torrent_status& ts);

    // The ratio as accumulated by this client alone, without the fallback above.
    double RealRatio(const libtorrent::torrent_status& ts);
}
