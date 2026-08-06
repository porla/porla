#pragma once

#include <string>
#include <vector>

namespace porla::Methods
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
}
