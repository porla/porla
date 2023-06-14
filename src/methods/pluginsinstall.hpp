#pragma once

#include "../config.hpp"
#include "method.hpp"
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
        explicit PluginsInstall(porla::Lua::Plugins::PluginEngine& plugin_engine);

    protected:
        void Invoke(const PluginsInstallReq& req, WriteCb<PluginsInstallRes> cb) override;

    private:
        porla::Lua::Plugins::PluginEngine& m_plugin_engine;
    };
}
