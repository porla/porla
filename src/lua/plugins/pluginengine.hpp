#pragma once

#include <filesystem>
#include <map>
#include <memory>
#include <optional>

#include <boost/asio.hpp>
#include <sqlite3.h>
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
        sqlite3*                           db;
        boost::asio::io_context&           io;
        std::vector<std::filesystem::path> plugins;
        ISession&                          session;
    };

    struct PluginState
    {
        std::filesystem::path   path;
        std::unique_ptr<Plugin> plugin;
    };

    struct PluginInstallOptions
    {
        std::optional<std::string> config;
        bool                       enable;
        std::filesystem::path      path;
    };

    class PluginEngine
    {
    public:
        explicit PluginEngine(const PluginEngineOptions& options);
        ~PluginEngine();

        void Install(const PluginInstallOptions& options, std::error_code& ec);
        void Uninstall(const std::string& name, std::error_code& ec);

        void UnloadAll();

    private:
        PluginEngineOptions m_options;
        std::map<std::string, PluginState> m_plugins;
    };
}
