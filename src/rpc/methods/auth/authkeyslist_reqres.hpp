#pragma once

#include <optional>
#include <string>
#include <vector>

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
}
