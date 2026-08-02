#include "../all.hpp"

#include "../../methods/sessions/sessionssettingsget_reqres.hpp"
#include "../utils.hpp"

namespace porla::Methods::Sessions
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        SessionsSettingsGetReq,
        id)

    NLOHMANN_JSONIFY_ALL_THINGS(
        SessionsSettingsGetRes,
        settings)
}
