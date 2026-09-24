#include "ratio.hpp"

#include <algorithm>

namespace
{
    double Divide(std::int64_t uploaded, std::int64_t downloaded)
    {
        if (downloaded <= 0)
        {
            return uploaded > 0 ? porla::Utils::MAX_RATIO : 0;
        }

        return std::min(
            static_cast<double>(uploaded) / static_cast<double>(downloaded),
            porla::Utils::MAX_RATIO);
    }
}

double porla::Utils::Ratio(const libtorrent::torrent_status& ts)
{
    const auto downloaded = ts.all_time_download < ts.total_done / 100
        ? ts.total_done
        : ts.all_time_download;

    return Divide(ts.all_time_upload, downloaded);
}

double porla::Utils::RealRatio(const libtorrent::torrent_status& ts)
{
    return Divide(ts.all_time_upload, ts.all_time_download);
}
