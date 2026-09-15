#include "../all.hpp"

#include "../../rpc/methods/auth/authinit_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Auth
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        AuthInitReq,
        username,
        password)

    void to_json(nlohmann::json& json, const AuthInitRes& res)
    {
        json = {};
    }
}
