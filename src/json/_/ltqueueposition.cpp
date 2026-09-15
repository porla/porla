#include "../all.hpp"

#include <libtorrent/torrent_handle.hpp>

namespace libtorrent
{
    void from_json(const nlohmann::json& j, queue_position_t& pos)
    {
        pos = queue_position_t{j.get<int>()};
    }

    void to_json(nlohmann::json& j, const queue_position_t& pos)
    {
        j = static_cast<int>(pos);
    }
}
