#include "../all.hpp"

#include "../../methods/sessions/sessionsresume_reqres.hpp"
#include "../utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        SessionsResumeReq,
        id)

    void to_json(nlohmann::json& j, const SessionsResumeRes& res)
    {
        j = true;
    }
}
