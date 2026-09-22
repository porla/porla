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
    class Plugin;
    struct PluginSource;

    struct PluginEngineOptions
    {
        Config&                     cfg;
        std::shared_ptr<CurlMulti>  curl_multi;
        sqlite3*                    db;
        uWS::App*                   http_server;
        std::weak_ptr<Rpc::JsonRpc> jsonrpc;
        boost::asio::io_context&    io;
        Sessions&                   sessions;
    };

    class PluginEngine
    {
    public:
        using CompletionCallback = std::function<void()>;

        explicit PluginEngine(const PluginEngineOptions& options);

        PluginEngine(const PluginEngine&)            = delete;
        PluginEngine& operator=(const PluginEngine&) = delete;

        ~PluginEngine();

        void SetCore(const PluginSource& sources);

        void Load(int id);
        void LoadAll();

        [[nodiscard]] const Plugin* Get(int id) const;

        void Reload(int id);
        void Unload(int id);

    private:
        std::unique_ptr<Plugin>                m_core_plugin;
        PluginEngineOptions                    m_options;
        std::map<int, std::unique_ptr<Plugin>> m_plugins;
    };
}
