#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace porla::Rpc::Methods::Auth
{
    struct AuthKeysCreateReq
    {
        std::string                 name;
        std::optional<std::int64_t> expires_at;
    };

    struct AuthKeysCreateRes
    {
        std::string id;
        std::string key;
    };
}
