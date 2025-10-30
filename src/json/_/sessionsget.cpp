#include "../all.hpp"

#include "../../methods/sessions/sessionsget_reqres.hpp"
#include "../utils.hpp"

namespace porla::Methods::Sessions
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        SessionsGetReq,
        id)

    NLOHMANN_JSONIFY_ALL_THINGS(
        SessionsGetRes::Session,
        id,
        name,
        is_default,
        metadata,
        settings)

    NLOHMANN_JSONIFY_ALL_THINGS(
        SessionsGetRes,
        session)
}
