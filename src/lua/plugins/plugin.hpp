#pragma once

#include <filesystem>
#include <memory>

#include <boost/asio.hpp>
#include <toml++/toml.h>

namespace porla
{
    class ISession;
}

namespace porla::Lua::Plugins
{
    struct PluginLoadOptions
    {
        toml::table              config;
        std::filesystem::path    dir;
        boost::asio::io_context& io;
        porla::ISession&         session;
    };

    class Plugin
    {
    public:
        static std::unique_ptr<Plugin> Load(const PluginLoadOptions& opts);

        virtual ~Plugin();

    private:
        class State;

        explicit Plugin(std::unique_ptr<State> state);

        std::unique_ptr<State> m_state;
    };
}
