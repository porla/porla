#pragma once

#include "../config.hpp"
#include "method.hpp"
#include "pluginsuninstall_reqres.hpp"

namespace porla::Lua::Plugins
{
    class PluginEngine;
}

namespace porla::Methods
{
    class PluginsUninstall : public Method<PluginsUninstallReq, PluginsUninstallRes>
    {
    public:
        explicit PluginsUninstall(porla::Lua::Plugins::PluginEngine& plugin_engine);

    protected:
        void Invoke(const PluginsUninstallReq& req, WriteCb<PluginsUninstallRes> cb) override;

    private:
        porla::Lua::Plugins::PluginEngine& m_plugin_engine;
    };
}
