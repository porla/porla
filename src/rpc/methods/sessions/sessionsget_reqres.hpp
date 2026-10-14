#pragma once

#include <map>
#include <string>

#include <libtorrent/settings_pack.hpp>

#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Sessions
{
    struct SessionsGetReq
    {
        int id;
    };

    struct SessionsGetRes
    {
        struct SessionState
        {
            bool  is_listening;
            bool  is_paused;
            int   torrents_total;
        };

        struct Session
        {
            int                                   id;
            std::string                           name;
            bool                                  is_default;
            std::map<std::string, nlohmann::json> metadata;
            std::optional<SessionState>           state;
        };

        Session session;
    };

    PORLA_JSON_REQUEST(SessionsGetReq, id)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
        SessionsGetRes::SessionState,
        is_listening,
        is_paused,
        torrents_total)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
        SessionsGetRes::Session,
        id,
        name,
        is_default,
        metadata,
        state)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
        SessionsGetRes,
        session)
}
