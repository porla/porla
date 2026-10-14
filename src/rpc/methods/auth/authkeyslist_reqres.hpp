#pragma once

#include <optional>
#include <string>
#include <vector>

#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Auth
{
    struct AuthKeysListReq
    {
    };

    struct AuthKeysListRes
    {
        struct Key
        {
            std::string                 id;
            std::string                 name;
            std::int64_t                created_at;
            std::optional<std::int64_t> expires_at;
        };

        std::vector<Key> keys;
    };

    PORLA_JSON_NO_PARAMS(AuthKeysListReq)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(AuthKeysListRes::Key, id, name, created_at, expires_at)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(AuthKeysListRes, keys)
}
