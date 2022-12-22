#include "eta.hpp"

std::chrono::seconds porla::Utils::ETA(const libtorrent::torrent_status& ts)
{
    auto eta = std::chrono::seconds(-1);

    if ((ts.total_wanted - ts.total_wanted_done > 0) && ts.download_payload_rate > 0)
    {
        eta = std::chrono::seconds((ts.total_wanted - ts.total_wanted_done) / ts.download_payload_rate);
    }

    return eta;
}
