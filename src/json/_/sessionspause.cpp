#include "../all.hpp"

#include "../../rpc/methods/sessions/sessionspause_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Sessions
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        SessionsPauseReq,
        id)

    void to_json(nlohmann::json& j, const SessionsPauseRes& res)
    {
    }
}
