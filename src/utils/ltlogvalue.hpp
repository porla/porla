#pragma once

#include <libtorrent/info_hash.hpp>

#include "hex.hpp"
#include "../logger.hpp"

namespace porla::Log
{
    template <>
    struct ValueOf<lt::info_hash_t>
    {
        static Value From(const lt::info_hash_t& hash)
        {
            return hash.has_v1()
                ? Utils::ToHex({ hash.v1.data(), static_cast<std::size_t>(hash.v1.size()) })
                : Utils::ToHex({ hash.v2.data(), static_cast<std::size_t>(hash.v2.size()) });
        }
    };
}
