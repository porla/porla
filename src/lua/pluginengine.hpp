#pragma once

#include <filesystem>
#include <map>
#include <memory>
#include <optional>

#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <sqlite3.h>
#include <toml++/toml.hpp>

namespace porla
{
    class Config;
    class Sessions;
}

namespace porla::Lua
{
    class Plugin;

    struct PluginEngineOptions
    {
        Config&                  config;
        sqlite3*                 db;
        boost::asio::io_context& io;
        Sessions&                sessions;
    };

    struct PluginState
    {
        std::string                                          type;
        std::unique_ptr<Plugin>                              plugin;
        std::optional<std::string>                           config;
        std::optional<std::map<std::string, nlohmann::json>> metadata;
    };

    class PluginEngine
    {
    public:
        explicit PluginEngine(PluginEngineOptions options);
        ~PluginEngine();

        void Configure(int id, const std::optional<std::string>& config);

        int InstallFromPath(
            const std::filesystem::path& path,
            std::optional<std::string> config,
            const nlohmann::json& metadata);

        int InstallFromArchive(
            const std::vector<char>& buffer,
            std::optional<std::string> config,
            const nlohmann::json& metadata);

        void LoadAll();

        std::map<int, PluginState>& Plugins();
        void Reload(int id);
        void Uninstall(int id);
        void UnloadAll();

    private:
        void Load(int id);
        void Unload(int id);

        PluginEngineOptions m_options;
        std::map<int, PluginState> m_plugins;
    };
}
