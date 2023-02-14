#pragma once

#include "../torrentclientdata.hpp"

#include "utils.hpp"

namespace porla
{
NLOHMANN_JSONIFY_ALL_THINGS(
    TorrentClientData,
    category,
    metadata,
    tags);
}
