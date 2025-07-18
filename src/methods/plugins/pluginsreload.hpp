#pragma once

#include "../method.hpp"
#include "pluginsreload_reqres.hpp"

namespace porla::Lua
{
    class PluginEngine;
}

namespace porla::Methods
{
    template <bool SSL> class PluginsReload : public Method<PluginsReloadReq, PluginsReloadRes, SSL>
    {
    public:
        explicit PluginsReload(porla::Lua::PluginEngine& plugin_engine);

    protected:
        void Invoke(const PluginsReloadReq& req, WriteCb<PluginsReloadRes, SSL> cb) override;

    private:
        porla::Lua::PluginEngine& m_plugin_engine;
    };
}
