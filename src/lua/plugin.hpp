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
#include <boost/asio/thread_pool.hpp>
#include <sqlite3.h>
#include <uWebSockets/App.h>

namespace porla
{
    class Config;
    class CurlMulti;
    class Sessions;
}

namespace porla::Rpc
{
    class JsonRpc;
}

namespace porla::Lua
{
    struct PluginLoadOptions
    {
        Config&                     cfg;
        std::shared_ptr<CurlMulti>  curl_multi;
        sqlite3*                    db;
        boost::asio::thread_pool&   hash_pool;
        uWS::App*                   http_server;
        std::weak_ptr<Rpc::JsonRpc> jsonrpc;
        boost::asio::io_context&    io;
        int                         plugin_id;
        porla::Sessions&            sessions;
    };

    class PluginSource;

    class Plugin
    {
    public:
        Plugin(const Plugin&)            = delete;
        Plugin(Plugin&&)                 = delete;
        Plugin& operator=(const Plugin&) = delete;
        Plugin& operator=(Plugin&&)      = delete;

        ~Plugin();

        PluginSource& Source() const;

        static std::unique_ptr<Plugin> Load(
            const PluginSource& source,
            const PluginLoadOptions& opts);

        static std::unique_ptr<Plugin> Load(
            const std::filesystem::path& path,
            const std::optional<std::string>& config,
            const PluginLoadOptions& opts);

    private:
        struct State;

        explicit Plugin(std::unique_ptr<State> state);

        std::unique_ptr<State> m_state;
    };
}
