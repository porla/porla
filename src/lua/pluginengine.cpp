#include "pluginengine.hpp"

#include <algorithm>
#include <set>
#include <utility>
#include <vector>

#include <boost/asio/post.hpp>
#include <boost/log/trivial.hpp>
#include <sqlite3.h>

#include "plugin.hpp"

#include "../config.hpp"
#include "../data/statement.hpp"
#include "../data/models/plugins.hpp"

namespace fs = std::filesystem;

using porla::Data::Models::Plugins;
using porla::Data::Statement;
using porla::Lua::Plugin;
using porla::Lua::PluginEngine;
using porla::Lua::PluginEngineOptions;

PluginEngine::PluginEngine(const PluginEngineOptions& options)
    : m_options(options)
{
}

PluginEngine::~PluginEngine()
{
    m_plugins.clear();
    m_core_plugin.reset();
}

void PluginEngine::SetCore(const PluginSource& sources)
{
    const PluginLoadOptions load_options{
        .cfg         = m_options.cfg,
        .curl_multi  = m_options.curl_multi,
        .db          = m_options.db,
        .hash_pool   = m_options.hash_pool,
        .http_server = m_options.http_server,
        .jsonrpc     = m_options.jsonrpc,
        .io          = m_options.io,
        .plugin_id   = 0,
        .sessions    = m_options.sessions
    };

    std::unique_ptr<Plugin> core_plugin = Plugin::Load(
        sources,
        load_options);

    if (core_plugin == nullptr)
    {
        throw std::runtime_error("Failed to load core Lua plugin");
    }

    m_core_plugin = std::move(core_plugin);
}

void PluginEngine::LoadAll()
{
    for (const auto& plugin : Plugins::List(m_options.db))
    {
        Load(plugin.id);
    }
}

void PluginEngine::Load(int id)
{
    if (m_plugins.find(id) != m_plugins.end())
    {
        BOOST_LOG_TRIVIAL(error) << "plugin[" << id << "] Already loaded";
        return;
    }

    const auto plugin = Plugins::GetById(m_options.db, id);

    if (!plugin.has_value())
    {
        BOOST_LOG_TRIVIAL(warning) << "plugin[" << id << "] Cannot load - not found";
        return;
    }

    const PluginLoadOptions load_options{
        .cfg         = m_options.cfg,
        .curl_multi  = m_options.curl_multi,
        .db          = m_options.db,
        .hash_pool   = m_options.hash_pool,
        .http_server = m_options.http_server,
        .jsonrpc     = m_options.jsonrpc,
        .io          = m_options.io,
        .plugin_id   = id,
        .sessions    = m_options.sessions
    };

    std::unique_ptr<Plugin> loaded_plugin = Plugin::Load(
        plugin->path,
        plugin->config,
        load_options);

    if (loaded_plugin == nullptr)
    {
        BOOST_LOG_TRIVIAL(warning) << "plugin[" << id << "] Failed to load";
        return;
    }

    m_plugins.emplace(id, std::move(loaded_plugin));

    BOOST_LOG_TRIVIAL(info) << "plugin[" << id << "] Loaded from " << plugin->path;
}

const Plugin* PluginEngine::Get(int id) const
{
    const auto it = m_plugins.find(id);
    return it == m_plugins.end() ? nullptr : it->second.get();
}

void PluginEngine::Reload(int id)
{
    if (m_plugins.contains(id))
    {
        Unload(id);
    }

    Load(id);
}

void PluginEngine::Unload(int id)
{
    if (m_plugins.contains(id))
    {
        m_plugins.erase(id);
        return;
    }

    BOOST_LOG_TRIVIAL(warning) << "plugin[" << id << "] Cannot unload - plugin not loaded";
}
