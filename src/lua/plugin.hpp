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
        static std::unique_ptr<Plugin> LoadFromArchive(
            const std::vector<char>& buffer,
            const std::optional<std::string>& config,
            const PluginLoadOptions& opts);

        static std::unique_ptr<Plugin> LoadFromPath(
            const std::filesystem::path& path,
            const std::optional<std::string>& config,
            const PluginLoadOptions& opts);

        virtual ~Plugin();

    private:
        class State;

        explicit Plugin(std::unique_ptr<State> state);

        std::unique_ptr<State> m_state;
    };
}
