#include "../all.hpp"

#include "../../methods/sessions/sessionsupdate_reqres.hpp"
#include "../utils.hpp"

namespace porla::Methods::Sessions
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        SessionsUpdateReq,
        id,
        metadata,
        settings)

    void to_json(nlohmann::json& j, const SessionsUpdateRes& res)
    {
        j = {};
    }
}
