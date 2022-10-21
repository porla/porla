#pragma once

#include <sstream>

#include <libtorrent/hex.hpp>
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
    static void from_json(const json& j, libtorrent::info_hash_t& ih)
    {
        if (j.is_string() && j.get<std::string>().size() == 40)
        {
            lt::sha1_hash h;
            lt::aux::from_hex({j.get<std::string>().c_str(),40}, h.data());
            ih = lt::info_hash_t(h);
        }
        else if (j.is_string() && j.size() == 68)
        {
        }
        else if (j.is_array() && j.size() == 2)
        {
        }
    }

    static void to_json(json& j, const libtorrent::info_hash_t& ih)
    {
        j = json::array();
        j.push_back(ih.has_v1() ? json(ToString(ih.v1)) : nullptr);
        j.push_back(ih.has_v2() ? json(ToString(ih.v2)) : nullptr);
    }
}
