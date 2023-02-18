#pragma once

#include <memory>

#include "../action.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Lua::Workflows::Actions
{
    struct TorrentMoveOptions
    {
        sol::object      path;
        porla::ISession& session;
    };

    class TorrentMove : public Action
    {
    public:
        explicit TorrentMove(const TorrentMoveOptions& opts);
        ~TorrentMove();

        void Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback) override;

    private:
        struct State;
        std::unique_ptr<State> m_state;
    };
}
