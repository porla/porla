#pragma once

#include <filesystem>

#include "../../config.hpp"
#include "../method.hpp"
#include "pluginsinstall_reqres.hpp"

namespace porla::Lua::Plugins
{
    class PluginEngine;
}

namespace porla::Methods
{
    class PluginsInstall : public Method<PluginsInstallReq, PluginsInstallRes>
    {
    public:
        explicit PluginsInstall(porla::Lua::Plugins::PluginEngine& plugin_engine, fs::path plugin_state_dir);

    protected:
        void Invoke(const PluginsInstallReq& req, WriteCb<PluginsInstallRes> cb) override;

    private:
        porla::Lua::Plugins::PluginEngine& m_plugin_engine;
        fs::path m_plugin_state_dir;
    };
}
