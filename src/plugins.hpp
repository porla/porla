#pragma once

#include <map>
#include <memory>
#include <optional>
#include <vector>

#include <boost/asio/io_context.hpp>
#include <boost/asio/thread_pool.hpp>
#include <sqlite3.h>
#include <uWebSockets/App.h>

#include "data/models/plugins.hpp"
#include "pluginevent.hpp"

namespace porla
{
    class Config;
    class CurlMulti;
    class Events;
    class KeyValue;
    class Presets;
    class Sessions;
    class Torrents;
}

namespace porla::Lua
{
    class Plugin;
    struct PluginLoadOptions;
    struct PluginSource;
}

namespace porla::Rpc
{
    class JsonRpc;
}

namespace porla
{
    struct PluginsOptions
    {
        Config&                     cfg;
        std::shared_ptr<CurlMulti>  curl_multi;
        sqlite3*                    db;
        Events&                     events;
        boost::asio::thread_pool&   hash_pool;
        uWS::App*                   http_server;
        std::weak_ptr<Rpc::JsonRpc> jsonrpc;
        boost::asio::io_context&    io;
        KeyValue&                   kv;
        Presets&                    presets;
        Sessions&                   sessions;
        Torrents&                   torrents;
    };

    class Plugins
    {
    public:
        using Plugin = Data::Models::Plugins::Plugin;

        explicit Plugins(const PluginsOptions& options);

        Plugins(const Plugins&)            = delete;
        Plugins& operator=(const Plugins&) = delete;

        ~Plugins();

        int Add(const Plugin& plugin);
        std::vector<Plugin> All() const;
        std::optional<Plugin> Get(int id) const;
        [[nodiscard]] const Lua::Plugin* Instance(int id) const;
        void LoadAll();
        void Reload(int id);
        void Remove(int id);
        void SetCore(const Lua::PluginSource& source);
        void Update(const Plugin& plugin);

    private:
        void Load(int id);
        Lua::PluginLoadOptions LoadOptions(int id) const;
        void Publish(const std::string& name, int id);
        void Unload(int id);

        PluginsOptions                              m_options;
        std::unique_ptr<Lua::Plugin>                m_core_plugin;
        std::map<int, std::unique_ptr<Lua::Plugin>> m_plugins;
    };
}
