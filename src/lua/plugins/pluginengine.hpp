#pragma once

#include <filesystem>
#include <memory>

#include <boost/asio.hpp>
#include <toml++/toml.h>

namespace porla::Lua::Plugins
{
    struct PluginEngineOptions
    {
        toml::table              config;
        boost::asio::io_context& io;
        std::filesystem::path    plugins_dir;
    };

    class PluginEngine
    {
    public:
        explicit PluginEngine(const PluginEngineOptions& options);
        ~PluginEngine();

    private:
        class State;
        std::unique_ptr<State> m_state;
    };
}
