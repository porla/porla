#pragma once

#include <map>
#include <memory>

#include "../action.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Lua::Workflows::Actions
{
    struct TorrentFlagsOptions
    {
        porla::ISession&         session;
        std::vector<std::string> set;
        std::vector<std::string> unset;
    };

    class TorrentFlags : public Action
    {
    public:
        explicit TorrentFlags(const TorrentFlagsOptions& opts);
        ~TorrentFlags();

        void Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback) override;

    private:
        TorrentFlagsOptions m_opts;
    };
}
