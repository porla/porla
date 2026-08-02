#include "../all.hpp"

#include "../../methods/sessions/sessionssettingsset_reqres.hpp"
#include "../utils.hpp"

namespace porla::Methods::Sessions
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        SessionsSettingsSetReq,
        id,
        settings)

    
    void to_json(nlohmann::json& j, const SessionsSettingsSetRes& res)
    {
        j = true;
    }
}
