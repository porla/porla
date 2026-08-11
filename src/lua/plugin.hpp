#pragma once

#include <chrono>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <boost/asio/io_context.hpp>
#include <sqlite3.h>
#include <uWebSockets/App.h>

namespace porla
{
    class Config;
    class CurlMulti;
    class Sessions;
}

namespace porla::Lua
{
    struct PluginLoadOptions
    {
        std::shared_ptr<CurlMulti> curl_multi;
        sqlite3*                   db;
        uWS::App*                  http_server;
        boost::asio::io_context&   io;
        porla::Sessions&           sessions;

        // How often we poll suspended coroutines to see whether they finished.
        std::chrono::milliseconds coroutine_poll_interval = std::chrono::milliseconds(250);

        // How long Unload() waits for outstanding coroutines before giving up on them.
        std::chrono::milliseconds unload_timeout = std::chrono::seconds(30);
    };

    class Plugin
    {
    public:
        struct Meta
        {
            std::optional<std::string> name;
            std::optional<std::string> version;
        };

        using UnloadCallback = std::function<void()>;

        Plugin(const Plugin&)            = delete;
        Plugin(Plugin&&)                 = delete;
        Plugin& operator=(const Plugin&) = delete;
        Plugin& operator=(Plugin&&)      = delete;

        ~Plugin();

        static std::unique_ptr<Plugin> LoadFromArchive(
            const std::vector<char>& buffer,
            const std::optional<std::string>& config,
            const PluginLoadOptions& opts);

        static std::unique_ptr<Plugin> LoadFromPath(
            const std::filesystem::path& path,
            const std::optional<std::string>& config,
            const PluginLoadOptions& opts);

        [[nodiscard]] std::optional<Meta> GetMeta() const;

        void Unload(UnloadCallback callback);

    private:
        struct State;

        explicit Plugin(std::shared_ptr<State> state);

        std::shared_ptr<State> m_state;
    };
}
