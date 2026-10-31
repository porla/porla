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
#include "lua/pluginmanifest.hpp"

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

        // "slug" in the plugin's manifest, else the plugin's file name without extension or its
        // directory name; lowercased, with "-" for anything outside a-z and 0-9
        static std::string SlugOf(const Plugin& plugin, const std::optional<Lua::PluginManifest>& manifest);

        // what SlugOf can't fix: a slug starts with a letter and has 2-32 characters
        static bool IsValidSlug(std::string_view slug);

        explicit Plugins(const PluginsOptions& options);

        Plugins(const Plugins&)            = delete;
        Plugins& operator=(const Plugins&) = delete;

        ~Plugins();

        int Add(const Plugin& plugin);
        std::vector<Plugin> All() const;

        // why the plugin's last load failed; nullopt when it's loaded
        std::optional<std::string> Error(int id) const;

        std::optional<Plugin> Get(int id) const;
        [[nodiscard]] const Lua::Plugin* Instance(int id) const;

        // the manifest the plugin's last load read; nullopt when it has none or it's invalid
        std::optional<Lua::PluginManifest> Manifest(int id) const;

        void LoadAll();
        void Reload(int id);
        void Remove(int id);
        void SetCore(const Lua::PluginSource& source);

        // a stored plugin's slug, from its last load's Manifest
        std::string Slug(const Plugin& plugin) const;

        // the id of the plugin with this slug (0 for the core plugin, "porla"), if any
        std::optional<int> SlugOwner(const std::string& slug, int except_id, bool loaded_only = false) const;

        void Update(const Plugin& plugin);

    private:
        struct LastLoad
        {
            std::optional<Lua::PluginManifest> manifest;
            std::optional<std::string>         error;
        };

        void Load(int id);
        void Publish(const std::string& name, int id);
        void Unload(int id);

        PluginsOptions                              m_options;
        std::unique_ptr<Lua::Plugin>                m_core_plugin;
        std::map<int, std::unique_ptr<Lua::Plugin>> m_plugins;
        std::map<int, LastLoad>                     m_loads;
    };
}
