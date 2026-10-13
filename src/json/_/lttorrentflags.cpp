#include "../all.hpp"

#include <stdexcept>
#include <unordered_set>

#include "../../utils/ltflags.hpp"

namespace libtorrent
{
    void from_json(const nlohmann::json& j, torrent_flags_t& flags)
    {
        flags = {};

        const auto input_flags = j.get<std::unordered_set<std::string>>();

        for (const auto& ifl : input_flags)
        {
            const auto found = porla::Utils::LtFlags::FindTorrentFlag(ifl);

            if (!found.has_value())
            {
                throw std::invalid_argument("unknown torrent flag: " + ifl);
            }

            flags |= *found;
        }
    }

    void to_json(nlohmann::json& j, const torrent_flags_t& flags)
    {
        j = nlohmann::json::array();

        for (const auto& [ name, flag ] : porla::Utils::LtFlags::AllTorrentFlags())
        {
            if ((flags & flag) == flag) { j.push_back(name); }
        }
    }
}
