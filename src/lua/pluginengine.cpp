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
    m_alive.reset();

    if (!m_plugins.empty())
    {
        BOOST_LOG_TRIVIAL(warning)
            << m_plugins.size() << " plugin(s) still loaded at engine shutdown - "
            << "their destroy functions will run synchronously and cannot yield. "
            << "Call UnloadAll() and wait for the callback before destroying the engine.";
    }

    if (!m_pending_unloads.empty())
    {
        BOOST_LOG_TRIVIAL(warning)
            << m_pending_unloads.size() << " plugin(s) were still unloading at engine shutdown";
    }

    m_plugins.clear();
    m_pending_unloads.clear();
}

void PluginEngine::Post(CompletionCallback callback) const
{
    if (!callback) return;

    boost::asio::post(m_options.io, [callback = std::move(callback)]() { callback(); });
}

int PluginEngine::InstallFromArchive(const std::vector<char>& buffer, std::optional<std::string> config, const nlohmann::json& metadata)
{
    auto install_stmt = Statement::Prepare(
        m_options.db,
        "INSERT INTO plugins (type, data, config, metadata) VALUES ('archive', $data, $config, $metadata)");
    install_stmt.Bind("$data", buffer);
    install_stmt.Bind("$config", config);
    install_stmt.Bind("$metadata", metadata.is_null()
        ? std::nullopt
        : std::optional(metadata.dump()));

    install_stmt.Execute();

    const auto plugin_id = static_cast<int>(sqlite3_last_insert_rowid(m_options.db));

    BOOST_LOG_TRIVIAL(info) << "plugin[" << plugin_id << "] installed with archive (" << buffer.size() << " bytes)";

    Load(plugin_id);

    return plugin_id;
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

    if (IsUnloading(id))
    {
        BOOST_LOG_TRIVIAL(error) << "plugin[" << id << "] Still unloading - cannot load yet";
        return;
    }

    const auto plugin = Plugins::GetById(m_options.db, id);

    if (!plugin.has_value())
    {
        BOOST_LOG_TRIVIAL(warning) << "plugin[" << id << "] Cannot load - not found";
        return;
    }

    const PluginLoadOptions load_options{
        .config     = m_options.config,
        .curl_multi = m_options.curl_multi,
        .io         = m_options.io,
        .sessions   = m_options.sessions
    };

    std::unique_ptr<Plugin> loaded_plugin = Plugin::LoadFromPath(
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
    return nullptr;
}

bool PluginEngine::IsUnloading(int id) const
{
    return std::any_of(
        m_pending_unloads.begin(),
        m_pending_unloads.end(),
        [id](const auto& iter) { return iter.second.id == id; });
}

void PluginEngine::Reload(int id, CompletionCallback callback)
{
    if (m_plugins.find(id) == m_plugins.end())
    {
        Load(id);
        Post(std::move(callback));
        return;
    }

    Unload(id, [this, alive = std::weak_ptr(m_alive), id, callback = std::move(callback)]() mutable
    {
        if (alive.expired()) return;

        Load(id);

        if (callback) callback();
    });
}

void PluginEngine::Unload(int id, CompletionCallback callback)
{
    const auto it = m_plugins.find(id);

    if (it == m_plugins.end())
    {
        BOOST_LOG_TRIVIAL(error) << "plugin[" << id << "] Cannot unload - plugin not loaded";
        Post(std::move(callback));
        return;
    }

    const auto token = m_next_unload_token++;

    const auto pending = m_pending_unloads.emplace(
        token,
        PendingUnload{
            .id = id,
            .plugin = std::move(it->second)
        }).first;

    m_plugins.erase(it);

    Plugin* plugin = pending->second.plugin.get();

    if (plugin == nullptr)
    {
        m_pending_unloads.erase(pending);
        Post(std::move(callback));
        return;
    }

    plugin->Unload(
        [this, alive = std::weak_ptr(m_alive), token, id, callback = std::move(callback)]() mutable
        {
            if (alive.expired())
            {
                // Engine (and the plugin with it) is already gone.
                return;
            }

            m_pending_unloads.erase(token);

            BOOST_LOG_TRIVIAL(info) << "plugin[" << id << "] unloaded";

            if (callback) callback();
        });
}

void PluginEngine::UnloadAll(CompletionCallback callback)
{
    std::vector<int> ids;
    ids.reserve(m_plugins.size());

    for (const auto& [id, _] : m_plugins)
    {
        ids.push_back(id);
    }

    if (ids.empty())
    {
        Post(std::move(callback));
        return;
    }

    auto remaining = std::make_shared<std::size_t>(ids.size());
    auto shared_cb = std::make_shared<CompletionCallback>(std::move(callback));

    for (const auto id : ids)
    {
        Unload(id, [remaining, shared_cb]()
        {
            if (--(*remaining) > 0) return;
            if (*shared_cb) (*shared_cb)();
        });
    }
}
