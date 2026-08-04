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

namespace fs = std::filesystem;

using porla::Data::Statement;
using porla::Lua::Plugin;
using porla::Lua::PluginEngine;
using porla::Lua::PluginEngineOptions;
using porla::Lua::PluginState;

namespace
{
    std::optional<std::map<std::string, nlohmann::json>> ParseMetadata(
        int id,
        const std::optional<std::string>& raw)
    {
        if (!raw || raw->empty())
        {
            return std::nullopt;
        }

        // Non-throwing parse - a corrupt metadata blob must not take down the engine.
        const nlohmann::json parsed = nlohmann::json::parse(*raw, nullptr, false);

        if (parsed.is_discarded() || !parsed.is_object())
        {
            BOOST_LOG_TRIVIAL(warning) << "plugin[" << id << "] Invalid metadata - ignoring";
            return std::nullopt;
        }

        std::map<std::string, nlohmann::json> metadata;

        for (const auto& [key, value] : parsed.items())
        {
            metadata.emplace(key, value);
        }

        return metadata;
    }
}

PluginEngine::PluginEngine(const PluginEngineOptions& options)
    : m_options(options)
{
}

PluginEngine::~PluginEngine()
{
    // Anything still in flight must not touch us after this point.
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

void PluginEngine::Configure(int id, const std::optional<std::string>& config, CompletionCallback callback)
{
    auto update_stmt = Statement::Prepare(
        m_options.db,
        "UPDATE plugins SET config = $config WHERE id = $id");
    update_stmt.Bind("$id",     id);
    update_stmt.Bind("$config", config);
    update_stmt.Execute();

    if (sqlite3_changes(m_options.db) < 1)
    {
        BOOST_LOG_TRIVIAL(warning) << "plugin[" << id << "] Not installed - config not stored";
        Post(std::move(callback));
        return;
    }

    if (m_plugins.find(id) == m_plugins.end())
    {
        Post(std::move(callback));
        return;
    }

    Reload(id, std::move(callback));
}

int PluginEngine::InstallFromPath(const fs::path& path, std::optional<std::string> config, const nlohmann::json& metadata)
{
    auto install_stmt = Statement::Prepare(
        m_options.db,
        "INSERT INTO plugins (type, data, config, metadata) VALUES ('path', $data, $config, $metadata)");
    install_stmt.Bind("$data", path.string());
    install_stmt.Bind("$config", config);
    install_stmt.Bind("$metadata", metadata.is_null()
        ? std::nullopt
        : std::optional(metadata.dump()));

    install_stmt.Execute();

    const auto plugin_id = static_cast<int>(sqlite3_last_insert_rowid(m_options.db));

    BOOST_LOG_TRIVIAL(info) << "plugin[" << plugin_id << "] installed with path " << path;

    Load(plugin_id);

    return plugin_id;
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
    std::set<int> plugin_ids;

    auto load_stmt = Statement::Prepare(
        m_options.db,
        "SELECT id FROM plugins ORDER BY id ASC");

    load_stmt.Step([&plugin_ids](const auto& row)
    {
        plugin_ids.insert(row.GetInt32("id"));
        return SQLITE_OK;
    });

    for (const auto plugin_id : plugin_ids)
    {
        Load(plugin_id);
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

    struct Row
    {
        bool                       found = false;
        std::string                type;
        std::string                path;
        std::vector<char>          buffer;
        std::optional<std::string> config;
        std::optional<std::string> metadata;
    };

    Row data;

    auto load_stmt = Statement::Prepare(
        m_options.db,
        "SELECT id,type,data,config,metadata FROM plugins WHERE id = $id");
    load_stmt.Bind("$id", id);

    load_stmt.Step(
        [&data](const auto& row)
        {
            data.found    = true;
            data.type     = row.GetStdString("type");
            data.config   = row.GetOptionalStdString("config");
            data.metadata = row.GetOptionalStdString("metadata");

            if (data.type == "path")
            {
                data.path = row.GetStdString("data");
            }
            else if (data.type == "archive")
            {
                data.buffer = row.GetBuffer("data");
            }

            return SQLITE_OK;
        });

    if (!data.found)
    {
        BOOST_LOG_TRIVIAL(warning) << "plugin[" << id << "] Not installed";
        return;
    }

    const PluginLoadOptions load_options{
        .config     = m_options.config,
        .curl_multi = m_options.curl_multi,
        .io         = m_options.io,
        .sessions   = m_options.sessions
    };

    std::unique_ptr<Plugin> plugin;

    if (data.type == "path")
    {
        BOOST_LOG_TRIVIAL(info) << "plugin[" << id << "] Loading from path " << data.path;
        plugin = Plugin::LoadFromPath(data.path, data.config, load_options);
    }
    else if (data.type == "archive")
    {
        BOOST_LOG_TRIVIAL(info) << "plugin[" << id << "] Loading from archive";
        plugin = Plugin::LoadFromArchive(data.buffer, data.config, load_options);
    }
    else
    {
        BOOST_LOG_TRIVIAL(warning) << "plugin[" << id << "] Invalid type: " << data.type;
        return;
    }

    if (plugin == nullptr)
    {
        BOOST_LOG_TRIVIAL(warning) << "plugin[" << id << "] Failed to load";
        return;
    }

    if (const auto meta = plugin->GetMeta(); meta && meta->name)
    {
        BOOST_LOG_TRIVIAL(info)
            << "plugin[" << id << "] Loaded " << *meta->name
            << " " << meta->version.value_or("(no version)");
    }

    m_plugins.emplace(id, PluginState{
        .type     = data.type,
        .plugin   = std::move(plugin),
        .config   = data.config,
        .metadata = ParseMetadata(id, data.metadata)
    });
}

std::map<int, PluginState>& PluginEngine::Plugins()
{
    return m_plugins;
}

const std::map<int, PluginState>& PluginEngine::Plugins() const
{
    return m_plugins;
}

const PluginState* PluginEngine::Get(int id) const
{
    const auto it = m_plugins.find(id);
    return it == m_plugins.end() ? nullptr : &it->second;
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

void PluginEngine::Uninstall(int id, CompletionCallback callback)
{
    auto uninstall_stmt = Statement::Prepare(
        m_options.db,
        "DELETE FROM plugins WHERE id = $id");
    uninstall_stmt.Bind("$id", id);
    uninstall_stmt.Execute();

    BOOST_LOG_TRIVIAL(info) << "plugin[" << id << "] uninstalled";

    if (m_plugins.find(id) == m_plugins.end())
    {
        Post(std::move(callback));
        return;
    }

    Unload(id, std::move(callback));
}

void PluginEngine::Unload(int id, CompletionCallback callback)
{
    const auto it = m_plugins.find(id);

    if (it == m_plugins.end())
    {
        BOOST_LOG_TRIVIAL(error) << "plugin[" << id << "] Not loaded";
        Post(std::move(callback));
        return;
    }

    const auto token = m_next_unload_token++;

    const auto pending = m_pending_unloads.emplace(
        token,
        PendingUnload{.id = id, .state = std::move(it->second)}).first;

    m_plugins.erase(it);

    Plugin* plugin = pending->second.state.plugin.get();

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
