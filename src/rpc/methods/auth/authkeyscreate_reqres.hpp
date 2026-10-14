#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "../../../json/utils.hpp"

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

    PORLA_JSON_REQUEST(AuthKeysCreateReq, name, expires_at)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(AuthKeysCreateRes, id, key)
}
