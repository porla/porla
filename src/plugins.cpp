#include "plugins.hpp"

#include <algorithm>
#include <filesystem>
#include <stdexcept>
#include <utility>

#include <boost/log/trivial.hpp>

#include "events.hpp"
#include "json/utils.hpp"
#include "lua/plugin.hpp"
#include "lua/pluginsource.hpp"

using porla::Plugins;
using porla::PluginsOptions;

using Model = porla::Data::Models::Plugins;

namespace
{
    // "slug" in the source's plugin.json. a broken plugin.json falls back to the name like a
    // missing one; plugins.get/list warn about it
    std::optional<std::string> DeclaredSlug(const porla::Lua::PluginSource& source)
    {
        const auto file = source.sources.find("plugin.json");

        if (file == source.sources.end())
        {
            return std::nullopt;
        }

        try
        {
            const auto json = porla::Json::Parse(std::string_view(file->second.data(), file->second.size()));
            const auto slug = json.find("slug");

            if (slug != json.end() && slug->is_string())
            {
                return slug->get<std::string>();
            }
        }
        catch (const std::exception&)
        {
        }

        return std::nullopt;
    }
}

std::string Plugins::SlugOf(const Plugin& plugin, const Lua::PluginSource* source)
{
    std::string name;

    if (const auto declared = source != nullptr ? DeclaredSlug(*source) : std::nullopt)
    {
        name = *declared;
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

    Model::Remove(m_options.db, id);

    Event event("plugin.removed", {
        { "plugin_id", id },
        { "plugin_path", plugin->path }
    });

    m_options.events.Publish(std::move(event));
}

void Plugins::SetCore(const Lua::PluginSource& source)
{
    std::unique_ptr<Lua::Plugin> core_plugin = Lua::Plugin::Load(source, std::nullopt, m_options, 0);

    if (core_plugin == nullptr)
    {
        throw std::runtime_error("Failed to load core Lua plugin");
    }

    m_core_plugin = std::move(core_plugin);
}

std::string Plugins::Slug(const Plugin& plugin) const
{
    if (const auto* instance = Instance(plugin.id))
    {
        return SlugOf(plugin, &instance->Source());
    }

    const auto source = Lua::PluginSource::Load(plugin.path);
    return SlugOf(plugin, source ? &*source : nullptr);
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

    const auto source = Lua::PluginSource::Load(plugin->path);

    if (!source.has_value())
    {
        BOOST_LOG_TRIVIAL(error) << "plugin[" << id << "] Failed to load plugin source from " << plugin->path;
        return;
    }

    // plugins added before slugs aren't checked yet: the first one loaded keeps a slug
    const auto slug = SlugOf(*plugin, &*source);

    if (!IsValidSlug(slug))
    {
        BOOST_LOG_TRIVIAL(error) << "plugin[" << id << "] Not loaded: invalid slug '" << slug << "'";
        return;
    }

    if (const auto owner = SlugOwner(slug, id, true))
    {
        BOOST_LOG_TRIVIAL(error) << "plugin[" << id << "] Not loaded: slug '" << slug << "' is used by plugin[" << *owner << "]";
        return;
    }

    std::unique_ptr<Lua::Plugin> loaded_plugin = Lua::Plugin::Load(
        *source,
        plugin->config,
        m_options,
        id);

    if (loaded_plugin == nullptr)
    {
        BOOST_LOG_TRIVIAL(warning) << "plugin[" << id << "] Failed to load";
        return;
    }

    m_plugins.emplace(id, std::move(loaded_plugin));

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
