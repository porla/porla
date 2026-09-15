#include "../all.hpp"

#include "../../rpc/methods/sessions/sessionssettingsset_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Sessions
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
