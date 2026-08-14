#include "../all.hpp"

#include "../../rpc/methods/sessions/sessionsadd_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Sessions
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        SessionsAddReq,
        name,
        metadata,
        settings,
        settings_base)

    NLOHMANN_JSONIFY_ALL_THINGS(
        SessionsAddRes,
        id);
}
