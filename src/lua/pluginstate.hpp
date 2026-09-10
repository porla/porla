#pragma once

#include <map>
#include <memory>

#include <boost/asio.hpp>
#include <boost/signals2.hpp>
#include <sol/sol.hpp>
#include <sqlite3.h>
#include <uWebSockets/App.h>

#include "../cron.hpp"
#include "../sessions.hpp"
#include "../timer.hpp"

namespace porla::Lua
{
    struct LuaState : public std::enable_shared_from_this<LuaState>
    {
        explicit LuaState(boost::asio::io_context& io, porla::Sessions& sessions)
            : io(io)
            , sessions(sessions)
        {
        }

        uWS::App*                                                 app;
        std::map<std::size_t, sol::protected_function>            callbacks;
        std::map<std::size_t, std::shared_ptr<CronSchedule>>      cron_schedules;
        sqlite3*                                                  db;
        std::vector<std::function<void()>>                        destructors;
        boost::asio::io_context&                                  io;
        std::size_t                                               next_id = 1;
        int                                                       plugin_id;
        porla::Sessions&                                          sessions;
        std::map<std::size_t, boost::signals2::scoped_connection> signals;
        std::map<std::size_t, std::shared_ptr<Timer>>             timers;
    };

    struct Cancellable
    {
        virtual void Cancel() = 0;
    };
}
