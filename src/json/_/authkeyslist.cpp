#include "../all.hpp"

#include "../../rpc/methods/auth/authkeyslist_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Auth
{
    void from_json(const nlohmann::json& j, AuthKeysListReq& req)
    {
    }

    NLOHMANN_JSONIFY_ALL_THINGS(
        AuthKeysListRes::Key,
        id,
        name,
        created_at,
        expires_at)

    NLOHMANN_JSONIFY_ALL_THINGS(
        AuthKeysListRes,
        keys)
}
