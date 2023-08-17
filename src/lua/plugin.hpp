#pragma once

#include <filesystem>
#include <memory>

#include <boost/asio.hpp>
#include <toml++/toml.h>

namespace porla
{
    class Config;
    class Sessions;
}

namespace porla::Lua
{
    struct PluginLoadOptions
    {
        Config&                    config;
        boost::asio::io_context&   io;
        std::filesystem::path      path;
        std::optional<std::string> plugin_config;
        porla::Sessions&           sessions;
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
