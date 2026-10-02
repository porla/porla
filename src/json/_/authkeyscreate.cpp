#include "../all.hpp"

#include "../../rpc/methods/auth/authkeyscreate_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Auth
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        AuthKeysCreateReq,
        name,
        expires_at)

    NLOHMANN_JSONIFY_ALL_THINGS(
        AuthKeysCreateRes,
        id,
        key)
}
