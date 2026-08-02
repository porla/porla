#include "../all.hpp"

#include "../../methods/sessions/sessionslist_reqres.hpp"
#include "../utils.hpp"

namespace porla::Methods
{
    void from_json(const nlohmann::json& j, SessionsListReq& req)
    {
    }

    NLOHMANN_JSONIFY_ALL_THINGS(
        SessionsListRes::Session,
        id,
        name,
        is_default,
        is_listening,
        is_paused,
        metadata,
        torrents_total)

    NLOHMANN_JSONIFY_ALL_THINGS(
        SessionsListRes,
        sessions)
}
