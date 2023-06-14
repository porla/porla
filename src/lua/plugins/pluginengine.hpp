#pragma once

#include <filesystem>
#include <memory>

#include <boost/asio.hpp>
#include <toml++/toml.h>

namespace porla
{
    class Config;
    class ISession;
}

namespace porla::Lua::Plugins
{
    class Plugin;

    struct PluginEngineOptions
    {
        Config&                            config;
        boost::asio::io_context&           io;
        std::vector<std::filesystem::path> plugins;
        ISession&                          session;
    };

    struct PluginState
    {
        std::unique_ptr<Plugin> plugin;
    };

    class PluginEngine
    {
    public:
        explicit PluginEngine(const PluginEngineOptions& options);
        ~PluginEngine();

        void UnloadAll();

    private:
        std::vector<PluginState> m_plugins;
    };
}
