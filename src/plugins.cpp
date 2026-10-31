#include "plugins.hpp"

#include <algorithm>
#include <filesystem>
#include <stdexcept>
#include <utility>

#include <boost/log/trivial.hpp>

#include "events.hpp"
#include "lua/plugin.hpp"
#include "lua/pluginsource.hpp"

using porla::Plugins;
using porla::PluginsOptions;

using Model = porla::Data::Models::Plugins;

std::string Plugins::SlugOf(const Plugin& plugin, const std::optional<Lua::PluginManifest>& manifest)
{
    std::string name;

    if (manifest.has_value() && manifest->slug.has_value())
    {
        name = *manifest->slug;
    }
    else
    {
        auto path = std::filesystem::path(plugin.path).lexically_normal();

        if (!path.has_filename())
        {
            path = path.parent_path();
        }

        name = path.extension() == ".lua" || path.extension() == ".zip"
            ? path.stem().string()
            : path.filename().string();
    }

    // lowercase, and anything else that can't be in a slug becomes "-"
    std::transform(
        name.begin(),
        name.end(),
        name.begin(),
        [](char c) -> char
        {
            if (c >= 'A' && c <= 'Z') { return c - 'A' + 'a'; }
            if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) { return c; }
            return '-';
        });

    return name;
}

std::optional<std::string> Plugins::Error(int id) const
{
    const auto it = m_loads.find(id);
    return it == m_loads.end() ? std::nullopt : it->second.error;
}

std::optional<porla::Lua::PluginManifest> Plugins::Manifest(int id) const
{
    const auto it = m_loads.find(id);
    return it == m_loads.end() ? std::nullopt : it->second.manifest;
}

bool Plugins::IsValidSlug(std::string_view slug)
{
    return slug.size() >= 2
        && slug.size() <= 32
        && slug[0] >= 'a'
        && slug[0] <= 'z';
}

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

    m_loads.erase(id);

    Model::Remove(m_options.db, id);

    Event event("plugin.removed", {
        { "plugin_id", id },
        { "plugin_path", plugin->path }
    });

    m_options.events.Publish(std::move(event));
}

void Plugins::SetCore(const Lua::PluginSource& source)
{
    try
    {
        m_core_plugin = Lua::Plugin::Load(source, std::nullopt, m_options, 0);
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("Failed to load core Lua plugin: ") + e.what());
    }
}

std::string Plugins::Slug(const Plugin& plugin) const
{
    return SlugOf(plugin, Manifest(plugin.id));
}

std::optional<int> Plugins::SlugOwner(const std::string& slug, int except_id, bool loaded_only) const
{
    if (slug == "porla")
    {
        return 0;
    }

    for (const auto& other : All())
    {
        if (other.id != except_id && (!loaded_only || m_plugins.contains(other.id)) && Slug(other) == slug)
        {
            return other.id;
        }
    }

    return std::nullopt;
}

std::optional<std::string_view> Plugins::File(const std::string& prefix, std::string_view slug, const std::string& path) const
{
    for (const auto& [ id, instance ] : m_plugins)
    {
        const auto last = m_loads.find(id);

        if (last == m_loads.end() || last->second.slug != slug)
        {
            continue;
        }

        // a lookup in the plugin's own files, so a path can't leave <prefix>
        const auto& sources = instance->Source().sources;
        const auto  file    = sources.find(prefix + path);

        if (file == sources.end())
        {
            return std::nullopt;
        }

        return std::string_view(file->second.data(), file->second.size());
    }

    return std::nullopt;
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

    auto& last = m_loads[id];
    last = {};

    try
    {
        const auto source = Lua::PluginSource::Load(plugin->path);

        if (!source.has_value())
        {
            throw std::runtime_error("Failed to load plugin source from " + plugin->path);
        }

        last.manifest = Lua::PluginManifest::FromSource(*source);

        // plugins added before slugs aren't checked yet: the first one loaded keeps a slug
        const auto slug = last.slug = SlugOf(*plugin, last.manifest);

        if (!IsValidSlug(slug))
        {
            throw std::runtime_error("Slug '" + slug + "' is invalid - the name must start with a letter and be 2-32 characters");
        }

        if (const auto owner = SlugOwner(slug, id, true))
        {
            throw std::runtime_error(
                "Slug '" + slug + "' is used by " + (*owner == 0 ? "the core plugin" : "plugin " + std::to_string(*owner)));
        }

        m_plugins.emplace(id, Lua::Plugin::Load(*source, plugin->config, m_options, id));
    }
    catch(const std::exception& e)
    {
        BOOST_LOG_TRIVIAL(error) << "plugin[" << id << "] Not loaded: " << e.what();

        // the reason without Lua's stack traceback, which stays in the log
        const std::string_view error = e.what();
        last.error = error.substr(0, error.find("\nstack traceback:"));

        return;
    }

    BOOST_LOG_TRIVIAL(info) << "plugin[" << id << "] Loaded from " << plugin->path;

    Publish("plugin.loaded", id);
}

void Plugins::Publish(const std::string& name, int id)
{
    Event event(name, {{ "plugin_id", id }});
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
