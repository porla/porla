#pragma once

#include <memory>

#include "../action.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Lua::Workflows::Actions
{
    struct TorrentPauseOptions
    {
        porla::ISession& session;
    };

    class TorrentPause : public Action
    {
    public:
        explicit TorrentPause(const TorrentPauseOptions& opts);
        ~TorrentPause();

        void Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback) override;

    private:
        struct State;
        std::unique_ptr<State> m_state;
    };
}
