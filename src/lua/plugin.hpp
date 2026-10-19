#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string>

namespace porla
{
    struct PluginsOptions;
}

namespace porla::Lua
{
    class PluginSource;

    class Plugin
    {
    public:
        Plugin(const Plugin&)            = delete;
        Plugin(Plugin&&)                 = delete;
        Plugin& operator=(const Plugin&) = delete;
        Plugin& operator=(Plugin&&)      = delete;

        ~Plugin();

        PluginSource& Source() const;

        static std::unique_ptr<Plugin> Load(
            const PluginSource& source,
            const std::optional<std::string>& config,
            const PluginsOptions& opts,
            int plugin_id);

        static std::unique_ptr<Plugin> Load(
            const std::filesystem::path& path,
            const std::optional<std::string>& config,
            const PluginsOptions& opts,
            int plugin_id);

    private:
        struct State;

        explicit Plugin(std::unique_ptr<State> state);

        std::unique_ptr<State> m_state;
    };
}
