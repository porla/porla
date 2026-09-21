#include "../all.hpp"

#include <sstream>

#include <libtorrent/info_hash.hpp>
#include <nlohmann/json.hpp>

#include "../../utils/hex.hpp"

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
    void from_json(const json& j, libtorrent::info_hash_t& ih)
    {
        // v1 info hash as a string
        if (j.is_string() && j.get<std::string>().size() == 40)
        {
            const auto hash = j.get<std::string>();

            lt::sha1_hash h;

            if (!porla::Utils::FromHex(hash, h.data(), hash.size()))
            {
                throw std::invalid_argument(
                    "info hash " + hash + " contains invalid hex characters");
            }

            ih = lt::info_hash_t(h);

            return;
        }

        // v2 info hash as a string
        if (j.is_string() && j.get<std::string>().size() == 64)
        {
            const auto hash = j.get<std::string>();

            lt::sha256_hash h;

            if (!porla::Utils::FromHex(hash, h.data(), hash.size()))
            {
                throw std::invalid_argument(
                    "info hash " + hash + " contains invalid hex characters");
            }

            ih = lt::info_hash_t(h);

            return;
        }

        if (j.is_array() && j.size() == 2)
        {
            // v1 info hash with null v2 hash
            if (j.at(0).is_string() && j.at(0).get<std::string>().size() == 40 && j.at(1).is_null())
            {
                const auto hash = j.at(0).get<std::string>();

                lt::sha1_hash h;
                
                if (!porla::Utils::FromHex(hash, h.data(), hash.size()))
                {
                    throw std::invalid_argument(
                        "info hash " + hash + " contains invalid hex characters");
                }

                ih = lt::info_hash_t(h);

                return;
            }

            // null v1 info hash with v2 info hash
            if (j.at(0).is_null() && j.at(1).is_string() && j.at(1).get<std::string>().size() == 64)
            {
                const auto hash = j.at(1).get<std::string>();

                lt::sha256_hash h;
                
                if (!porla::Utils::FromHex(hash, h.data(), hash.size()))
                {
                    throw std::invalid_argument(
                        "info hash " + hash + " contains invalid hex characters");
                }

                ih = lt::info_hash_t(h);

                return;
            }

            // both v1 and v2 hashes
            if (j.at(0).is_string() && j[0].get<std::string>().size() == 40
                && j.at(1).is_string() && j[1].get<std::string>().size() == 64)
            {
                const auto h1 = j.at(0).get<std::string>();
                const auto h2 = j.at(1).get<std::string>();

                lt::sha1_hash v1;

                if (!porla::Utils::FromHex(h1, v1.data(), h1.size()))
                {
                    throw std::invalid_argument(
                        "info hash " + h1 + " contains invalid hex characters");
                }

                lt::sha256_hash v2;

                if (!porla::Utils::FromHex(h2, v2.data(), h2.size()))
                {
                    throw std::invalid_argument(
                        "info hash " + h2 + " contains invalid hex characters");
                }

                ih = lt::info_hash_t(v1, v2);

                return;
            }
        }
    }

    void to_json(json& j, const libtorrent::info_hash_t& ih)
    {
        j = json::array();
        j.push_back(ih.has_v1() ? json(ToString(ih.v1)) : nullptr);
        j.push_back(ih.has_v2() ? json(ToString(ih.v2)) : nullptr);
    }
}
