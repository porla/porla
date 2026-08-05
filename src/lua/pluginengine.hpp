#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <boost/asio/io_context.hpp>
#include <nlohmann/json.hpp>
#include <sqlite3.h>

namespace porla
{
    class Config;
    class CurlMulti;
    class Sessions;
}

namespace porla::Lua
{
    class Plugin;

    struct PluginEngineOptions
    {
        Config&                    config;
        std::shared_ptr<CurlMulti> curl_multi;
        sqlite3*                   db;
        boost::asio::io_context&   io;
        Sessions&                  sessions;
    };

    class PluginEngine
    {
    public:
        // Every completion callback is invoked via boost::asio::post, never inline, so
        // it is always safe to mutate the engine from inside one.
        using CompletionCallback = std::function<void()>;

        explicit PluginEngine(const PluginEngineOptions& options);

        PluginEngine(const PluginEngine&)            = delete;
        PluginEngine& operator=(const PluginEngine&) = delete;

        ~PluginEngine();

        // Persists the config, then reloads the plugin if it is currently loaded.
        void Configure(int id, const std::optional<std::string>& config, CompletionCallback callback = {});

        int InstallFromArchive(
            const std::vector<char>& buffer,
            std::optional<std::string> config,
            const nlohmann::json& metadata);

        void Load(int id);
        void LoadAll();

        [[nodiscard]] const Plugin* Get(int id) const;

        [[nodiscard]] bool IsUnloading(int id) const;

        void Reload(int id, CompletionCallback callback = {});
        void Uninstall(int id, CompletionCallback callback = {});
        void Unload(int id, CompletionCallback callback = {});
        void UnloadAll(CompletionCallback callback = {});

    private:
        void Post(CompletionCallback callback) const;

        struct PendingUnload
        {
            int                     id;
            std::unique_ptr<Plugin> plugin;
        };

        PluginEngineOptions                    m_options;
        std::map<int, std::unique_ptr<Plugin>> m_plugins;

        // Plugins that have been taken out of m_plugins but are still finishing their
        // destroy coroutine. They must stay alive until Plugin::Unload calls back.
        std::map<std::uint64_t, PendingUnload> m_pending_unloads;
        std::uint64_t                          m_next_unload_token = 1;

        // Lets posted completion handlers detect that the engine is gone.
        std::shared_ptr<void> m_alive = std::make_shared<char>();
    };
}
