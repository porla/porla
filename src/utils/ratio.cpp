#include "ratio.hpp"

double porla::Utils::Ratio(const libtorrent::torrent_status &ts, bool real)
{
    if (ts.all_time_download > 0)
    {
        if (real)
        {
            return (double)ts.all_time_upload / (double)ts.all_time_download;
        }

        const auto total_size = ts.total + ts.total_done;
        return total_size / ts.all_time_download;
    }

    return 0;
}
