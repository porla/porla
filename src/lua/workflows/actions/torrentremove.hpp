#pragma once

#include <memory>

#include "../action.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Lua::Workflows::Actions
{
    struct TorrentRemoveOptions
    {
        porla::ISession& session;
        bool             remove_files;
    };

    class TorrentRemove : public Action
    {
    public:
        explicit TorrentRemove(TorrentRemoveOptions opts);
        ~TorrentRemove();

        void Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback) override;

    private:
        class State;
        std::shared_ptr<State> m_state;
    };
}
