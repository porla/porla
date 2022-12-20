#include "ratio.hpp"

double porla::Utils::Ratio(const libtorrent::torrent_status &ts)
{
    if (ts.all_time_download > 0)
    {
        return (double) ts.all_time_upload / (double) ts.all_time_download;
    }

    return 0;
}
