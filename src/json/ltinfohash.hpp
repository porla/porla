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
        // v1 info hash as a string
        if (j.is_string() && j.get<std::string>().size() == 40)
        {
            lt::sha1_hash h;
            lt::aux::from_hex({j.get<std::string>().c_str(),40}, h.data());
            ih = lt::info_hash_t(h);

            return;
        }

        // v2 info hash as a string
        if (j.is_string() && j.get<std::string>().size() == 64)
        {
            lt::sha256_hash h;
            lt::aux::from_hex({j.get<std::string>().c_str(),64}, h.data());
            ih = lt::info_hash_t(h);

            return;
        }

        if (j.is_array() && j.size() == 2)
        {
            // v1 info hash with null v2 hash
            if (j.at(0).is_string() && j.at(0).get<std::string>().size() == 40 && j.at(1).is_null())
            {
                lt::sha1_hash h;
                lt::aux::from_hex({j[0].get<std::string>().c_str(),40}, h.data());
                ih = lt::info_hash_t(h);

                return;
            }

            // null v1 info hash with v2 info hash
            if (j.at(0).is_null() && j.at(1).is_string() && j.at(1).get<std::string>().size() == 64)
            {
                lt::sha256_hash h;
                lt::aux::from_hex({j[1].get<std::string>().c_str(),64}, h.data());
                ih = lt::info_hash_t(h);

                return;
            }

            // both v1 and v2 hashes
            if (j.at(0).is_string() && j[0].get<std::string>().size() == 40
                && j.at(1).is_string() && j[1].get<std::string>().size() == 64)
            {
                lt::sha1_hash v1;
                lt::aux::from_hex({j[0].get<std::string>().c_str(),40}, v1.data());

                lt::sha256_hash v2;
                lt::aux::from_hex({j[1].get<std::string>().c_str(),64}, v2.data());

                ih = lt::info_hash_t(v1, v2);

                return;
            }
        }
    }

    static void to_json(json& j, const libtorrent::info_hash_t& ih)
    {
        j = json::array();
        j.push_back(ih.has_v1() ? json(ToString(ih.v1)) : nullptr);
        j.push_back(ih.has_v2() ? json(ToString(ih.v2)) : nullptr);
    }
}
