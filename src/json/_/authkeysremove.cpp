#include "../all.hpp"

#include "../../rpc/methods/auth/authkeysremove_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Auth
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        AuthKeysRemoveReq,
        id)

    void to_json(nlohmann::json& j, const AuthKeysRemoveRes& res)
    {
        j = {};
    }
}
