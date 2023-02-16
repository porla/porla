#pragma once

#include <memory>

#include <boost/asio.hpp>

namespace porla
{
    class ISession;
}

namespace porla::Lua::Workflows
{
    class Action;

    struct ActionBuilderOptions
    {
        boost::asio::io_context& io;
        porla::ISession&         session;
    };

    struct ActionBuilder
    {
        virtual std::shared_ptr<Action> Build(const ActionBuilderOptions& opts) = 0;
    };
}