#include "../all.hpp"

#include "../../rpc/methods/sessions/sessionsupdate_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Sessions
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        SessionsUpdateReq,
        id,
        name,
        is_default,
        metadata,
        timer_dht_stats,
        timer_save_state,
        timer_session_stats,
        timer_torrent_updates)

    void to_json(nlohmann::json& j, const SessionsUpdateRes& res)
    {
        j = {};
    }
}
