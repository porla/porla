#pragma once

#include "../torrentclientdata.hpp"

#include "mediainfo.hpp"
#include "utils.hpp"

namespace porla
{
NLOHMANN_JSONIFY_ALL_THINGS(
    TorrentClientData,
    category,
    mediainfo,
    mediainfo_file_pieces_completed,
    mediainfo_file_pieces_wanted,
    metadata,
    tags);
}
