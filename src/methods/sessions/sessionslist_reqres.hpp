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
        struct Session
        {
            int                                   id;
            std::string                           name;
            bool                                  is_default;
            bool                                  is_listening;
            bool                                  is_paused;
            std::map<std::string, nlohmann::json> metadata;
            int                                   torrents_total;
        };

        std::vector<Session> sessions;
    };
}
