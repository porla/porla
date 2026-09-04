#include "../all.hpp"

#include "../../rpc/methods/sessions/sessionssettingsget_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Sessions
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        SessionsSettingsGetReq,
        id)

    NLOHMANN_JSONIFY_ALL_THINGS(
        SessionsSettingsGetRes,
        settings)
}
