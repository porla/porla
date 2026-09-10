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
    for (const auto& [ id, _ ] : m_plugins)
    {
        Unload(id);
    }

    m_plugins.clear();
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
        .curl_multi  = m_options.curl_multi,
        .db          = m_options.db,
        .http_server = m_options.http_server,
        .io          = m_options.io,
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

    if (const auto meta = m_plugins.at(id)->GetMeta(); meta && meta->name)
    {
        BOOST_LOG_TRIVIAL(info)
            << "plugin[" << id << "] Loaded " << *meta->name
            << " " << meta->version.value_or("(no version)");
    }
}

const Plugin* PluginEngine::Get(int id) const
{
    const auto it = m_plugins.find(id);
    return it == m_plugins.end() ? nullptr : it->second.get();
}

void PluginEngine::Reload(int id)
{
    if (m_plugins.find(id) == m_plugins.end())
    {
        Load(id);
        return;
    }

    Unload(id);

    Load(id);
}

void PluginEngine::Unload(int id)
{
    const auto it = m_plugins.find(id);

    if (it == m_plugins.end())
    {
        BOOST_LOG_TRIVIAL(warning) << "plugin[" << id << "] Cannot unload - plugin not loaded";
        return;
    }

    it->second->Unload();

    m_plugins.erase(it);
}
