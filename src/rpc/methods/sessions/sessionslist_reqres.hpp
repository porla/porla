#pragma once

#include <string>
#include <vector>

#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Sessions
{
    struct SessionsListReq
    {
    };

    struct SessionsListRes
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

        std::vector<Session> sessions;
    };

    PORLA_JSON_NO_PARAMS(SessionsListReq)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
        SessionsListRes::SessionState,
        is_listening,
        is_paused,
        torrents_total)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
        SessionsListRes::Session,
        id,
        name,
        is_default,
        metadata,
        state)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
        SessionsListRes,
        sessions)
}
