#pragma once

#include "../method.hpp"
#include "pluginsuninstall_reqres.hpp"

namespace porla::Lua
{
    class PluginEngine;
}

namespace porla::Methods
{
    class PluginsUninstall : public Method<PluginsUninstallReq, PluginsUninstallRes>
    {
    public:
        explicit PluginsUninstall(porla::Lua::PluginEngine& plugin_engine);

    protected:
        void Invoke(const PluginsUninstallReq& req, WriteCb<PluginsUninstallRes> cb) override;

    private:
        porla::Lua::PluginEngine& m_plugin_engine;
    };
}
