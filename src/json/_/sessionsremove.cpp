#include "../all.hpp"

#include "../../rpc/methods/sessions/sessionsremove_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Sessions
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        SessionsRemoveReq,
        id)

    void to_json(nlohmann::json& j, const SessionsRemoveRes& res)
    {
        j = {};
    }
}
