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
        ISession&                          session;
    };

    struct PluginState
    {
        std::optional<std::string> config;
        std::filesystem::path      path;
        std::unique_ptr<Plugin>    plugin;
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
        explicit PluginEngine(PluginEngineOptions options);
        ~PluginEngine();

        void Configure(const std::string& name, const std::optional<std::string>& config);
        void Install(const PluginInstallOptions& options, std::error_code& ec);
        std::map<std::string, PluginState>& Plugins();
        void Reload(const std::string& name);
        void Uninstall(const std::string& name, std::error_code& ec);

        void UnloadAll();

    private:
        PluginEngineOptions m_options;
        std::map<std::string, PluginState> m_plugins;
        std::vector<std::unique_ptr<Plugin>> m_workflows;
    };
}
