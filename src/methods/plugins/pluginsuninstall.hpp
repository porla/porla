#pragma once

#include "../method.hpp"
#include "pluginsuninstall_reqres.hpp"

namespace porla::Lua
{
    class PluginEngine;
}

namespace porla::Methods
{
    template <bool SSL> class PluginsUninstall : public Method<PluginsUninstallReq, PluginsUninstallRes, SSL>
    {
    public:
        explicit PluginsUninstall(porla::Lua::PluginEngine& plugin_engine);

    protected:
        void Invoke(const PluginsUninstallReq& req, WriteCb<PluginsUninstallRes, SSL> cb) override;

    private:
        porla::Lua::PluginEngine& m_plugin_engine;
    };
}
