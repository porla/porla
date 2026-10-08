#include "plugins.hpp"

#include <stdexcept>
#include <utility>

#include <boost/log/trivial.hpp>

#include "events.hpp"
#include "lua/plugin.hpp"

using porla::PluginEvent;
using porla::Plugins;
using porla::PluginsOptions;

using Model = porla::Data::Models::Plugins;

Plugins::Plugins(const PluginsOptions& options)
    : m_options(options)
{
}

Plugins::~Plugins()
{
    m_plugins.clear();
    m_core_plugin.reset();
}

int Plugins::Add(const Plugin& plugin)
{
    const int id = Model::Insert(m_options.db, plugin);

    BOOST_LOG_TRIVIAL(info) << "plugin[" << id << "] Added with path " << plugin.path;

    Publish("plugin.added", id);

    Load(id);

    return id;
}

std::vector<Plugins::Plugin> Plugins::All() const
{
    return Model::List(m_options.db);
}

std::optional<Plugins::Plugin> Plugins::Get(int id) const
{
    return Model::GetById(m_options.db, id);
}

const porla::Lua::Plugin* Plugins::Instance(int id) const
{
    const auto it = m_plugins.find(id);
    return it == m_plugins.end() ? nullptr : it->second.get();
}

void Plugins::LoadAll()
{
    for (const auto& plugin : All())
    {
        Load(plugin.id);
    }
}

void Plugins::Reload(int id)
{
    Unload(id);
    Load(id);
}

void Plugins::Remove(int id)
{
    const auto plugin = Get(id);

    if (!plugin.has_value())
    {
        return;
    }

    Unload(id);

    Model::Remove(m_options.db, id);

    PluginEvent event("plugin.removed", {{ "plugin_path", plugin->path }});
    event.plugin_id = id;

    m_options.events.Publish(std::move(event));
}

void Plugins::SetCore(const Lua::PluginSource& source)
{
    std::unique_ptr<Lua::Plugin> core_plugin = Lua::Plugin::Load(source, LoadOptions(0));

    if (core_plugin == nullptr)
    {
        throw std::runtime_error("Failed to load core Lua plugin");
    }

    m_core_plugin = std::move(core_plugin);
}

void Plugins::Update(const Plugin& plugin)
{
    Model::Update(m_options.db, plugin);

    Publish("plugin.updated", plugin.id);

    Reload(plugin.id);
}

void Plugins::Load(int id)
{
    if (m_plugins.contains(id))
    {
        BOOST_LOG_TRIVIAL(error) << "plugin[" << id << "] Already loaded";
        return;
    }

    const auto plugin = Get(id);

    if (!plugin.has_value())
    {
        BOOST_LOG_TRIVIAL(warning) << "plugin[" << id << "] Cannot load - not found";
        return;
    }

    std::unique_ptr<Lua::Plugin> loaded_plugin = Lua::Plugin::Load(
        plugin->path,
        plugin->config,
        LoadOptions(id));

    if (loaded_plugin == nullptr)
    {
        BOOST_LOG_TRIVIAL(warning) << "plugin[" << id << "] Failed to load";
        return;
    }

    m_plugins.emplace(id, std::move(loaded_plugin));

    BOOST_LOG_TRIVIAL(info) << "plugin[" << id << "] Loaded from " << plugin->path;

    Publish("plugin.loaded", id);
}

porla::Lua::PluginLoadOptions Plugins::LoadOptions(int id) const
{
    return Lua::PluginLoadOptions{
        .cfg         = m_options.cfg,
        .curl_multi  = m_options.curl_multi,
        .db          = m_options.db,
        .events      = m_options.events,
        .kv          = m_options.kv,
        .hash_pool   = m_options.hash_pool,
        .http_server = m_options.http_server,
        .jsonrpc     = m_options.jsonrpc,
        .io          = m_options.io,
        .plugin_id   = id,
        .presets     = m_options.presets,
        .sessions    = m_options.sessions,
        .torrents    = m_options.torrents
    };
}

void Plugins::Publish(const std::string& name, int id)
{
    PluginEvent event(name);
    event.plugin_id = id;

    m_options.events.Publish(std::move(event));
}

void Plugins::Unload(int id)
{
    if (m_plugins.erase(id) == 0)
    {
        return;
    }

    Publish("plugin.unloaded", id);
}
