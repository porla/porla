#pragma once

#include <libtorrent/torrent_status.hpp>

namespace porla::Utils
{
    std::chrono::seconds ETA(const libtorrent::torrent_status& ts);
}
