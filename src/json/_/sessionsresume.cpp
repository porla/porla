#include "../all.hpp"

#include "../../rpc/methods/sessions/sessionsresume_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Sessions
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        SessionsResumeReq,
        id)

    void to_json(nlohmann::json& j, const SessionsResumeRes& res)
    {
        j = true;
    }
}
