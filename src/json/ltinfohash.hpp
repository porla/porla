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
        j = json::array();
        j.push_back(ih.has_v1() ? json(ToString(ih.v1)) : nullptr);
        j.push_back(ih.has_v2() ? json(ToString(ih.v2)) : nullptr);
    }
}
