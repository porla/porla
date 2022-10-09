#pragma once

#include <sstream>

#include <libtorrent/info_hash.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

template<typename T>
static std::string ToString(const T &hash)
{
    std::stringstream ss;
    ss << hash;
    return ss.str();
}

namespace libtorrent
{
    static void to_json(json& j, const libtorrent::info_hash_t& ih)
    {
        j = {
            {"v1", ih.has_v1() ? ToString(ih.v1) : std::string()},
            {"v2", ih.has_v2() ? ToString(ih.v2) : std::string()}
        };
    }
}
