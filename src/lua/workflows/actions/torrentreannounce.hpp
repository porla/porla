#pragma once

#include <map>
#include <memory>

#include "../action.hpp"

namespace boost::asio
{
    class io_context;
}

namespace porla
{
    class ISession;
}

namespace porla::Lua::Workflows::Actions
{
    struct TorrentReannounceOptions
    {
        int                      max_tries;
        porla::ISession&         session;
        int                      timeout;
    };

    class TorrentReannounce : public Action
    {
    public:
        explicit TorrentReannounce(const TorrentReannounceOptions& opts);
        ~TorrentReannounce();

        void Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback) override;

    private:
        struct State;
        std::unique_ptr<State> m_state;
    };
}
