#include "../all.hpp"

#include "../../methods/sessions/sessionssettingslist_reqres.hpp"
#include "../utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        SessionsSettingsListReq,
        id,
        keys)

    NLOHMANN_JSONIFY_ALL_THINGS(
        SessionsSettingsListRes,
        settings)
}
