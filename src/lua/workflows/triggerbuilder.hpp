#pragma once

#include <memory>
#include <memory>

#include <boost/asio.hpp>
#include <sol/sol.hpp>

namespace porla
{
    class ISession;
}

namespace porla::Lua::Workflows
{
    class Trigger;

    struct TriggerBuilderOptions
    {
        std::vector<sol::object> actions;
        boost::asio::io_context& io;
        sol::state&              lua;
        porla::ISession&         session;
    };

    struct TriggerBuilder
    {
        virtual std::shared_ptr<Trigger> Build(const TriggerBuilderOptions& opts) = 0;
    };
}
