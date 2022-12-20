#pragma once

#include <libtorrent/torrent_status.hpp>

namespace porla::Utils
{
    double Ratio(const libtorrent::torrent_status& ts);
}
