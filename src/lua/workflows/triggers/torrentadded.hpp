#pragma once

#include <boost/asio.hpp>
#include <sol/sol.hpp>

#include "../trigger.hpp"

namespace porla { class ISession; }

namespace porla::Lua::Workflows::Triggers
{
    struct TorrentAddedOptions
    {
        std::vector<sol::object> actions;
        sol::object              filter;
        boost::asio::io_context& io;
        sol::state&              lua;
        porla::ISession&         session;
    };

    class TorrentAdded : public Trigger
    {
    public:
        explicit TorrentAdded(const TorrentAddedOptions& opts);
        ~TorrentAdded();

    private:
        class State;
        std::unique_ptr<State> m_state;
    };
}
