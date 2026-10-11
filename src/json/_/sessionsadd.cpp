#include "../all.hpp"

#include "../../rpc/methods/sessions/sessionsadd_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Sessions
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        SessionsAddReq,
        name,
        metadata,
        settings,
        settings_base,
        timer_dht_stats,
        timer_save_state,
        timer_session_stats,
        timer_torrent_updates)

    NLOHMANN_JSONIFY_ALL_THINGS(
        SessionsAddRes,
        id);
}
