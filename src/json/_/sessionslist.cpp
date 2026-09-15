#include "../all.hpp"

#include "../../rpc/methods/sessions/sessionslist_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Sessions
{
    void from_json(const nlohmann::json& j, SessionsListReq& req)
    {
    }

    NLOHMANN_JSONIFY_ALL_THINGS(
        SessionsListRes::Session,
        id,
        name,
        is_default,
        metadata,
        state)

    NLOHMANN_JSONIFY_ALL_THINGS(
        SessionsListRes::SessionState,
        is_listening,
        is_paused,
        torrents_total)

    NLOHMANN_JSONIFY_ALL_THINGS(
        SessionsListRes,
        sessions)
}
