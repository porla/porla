#pragma once

#include <filesystem>
#include <memory>
#include <variant>
#include <vector>

#include <boost/asio.hpp>
#include <toml++/toml.hpp>

namespace porla
{
    class Config;
    class Sessions;
}

namespace porla::Lua
{
    struct PluginLoadOptions
    {
        Config&                                                config;
        boost::asio::io_context&                               io;
        porla::Sessions&                                       sessions;
    };

    class Plugin
    {
    public:
        struct Manifest
        {
            std::optional<std::string> name;
            std::optional<std::string> version;
        };

        static std::unique_ptr<Plugin> LoadFromArchive(
            const std::vector<char>& buffer,
            const std::optional<std::string>& config,
            const PluginLoadOptions& opts);

        static std::unique_ptr<Plugin> LoadFromPath(
            const std::filesystem::path& path,
            const std::optional<std::string>& config,
            const PluginLoadOptions& opts);

        std::optional<Manifest> GetManifest();

        virtual ~Plugin();

    private:
        class State;

        explicit Plugin(std::unique_ptr<State> state);

        std::unique_ptr<State> m_state;
    };
}
