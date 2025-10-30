#include "../all.hpp"

#include "../../methods/sessions/sessionslist_reqres.hpp"
#include "../utils.hpp"

namespace porla::Methods
{
    void from_json(const nlohmann::json& j, SessionsListReq& req)
    {
    }

    NLOHMANN_JSONIFY_ALL_THINGS(
        SessionsListRes::Item,
        id,
        name,
        is_default,
        metadata,
        torrents_total)

    NLOHMANN_JSONIFY_ALL_THINGS(
        SessionsListRes,
        sessions)
}
