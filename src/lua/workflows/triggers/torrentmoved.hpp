#pragma once

#include <boost/asio.hpp>
#include <sol/sol.hpp>

#include "../trigger.hpp"

namespace porla { class ISession; }

namespace porla::Lua::Workflows::Triggers
{
    struct TorrentMovedOptions
    {
        std::vector<sol::object> actions;
        boost::asio::io_context& io;
        sol::state&              lua;
        porla::ISession&         session;
    };

    class TorrentMoved : public Trigger
    {
    public:
        explicit TorrentMoved(const TorrentMovedOptions& opts);
        ~TorrentMoved();

    private:
        class State;
        std::unique_ptr<State> m_state;
    };
}
