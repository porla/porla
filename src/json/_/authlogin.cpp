#include "../all.hpp"

#include "../../rpc/methods/auth/authlogin_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Auth
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        AuthLoginReq,
        username,
        password)

    NLOHMANN_JSONIFY_ALL_THINGS(
        AuthLoginRes,
        token)
}
