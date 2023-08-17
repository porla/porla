#pragma once

#include "../method.hpp"
#include "pluginsreload_reqres.hpp"

namespace porla::Lua
{
    class PluginEngine;
}

namespace porla::Methods
{
    class PluginsReload : public Method<PluginsReloadReq, PluginsReloadRes>
    {
    public:
        explicit PluginsReload(porla::Lua::PluginEngine& plugin_engine);

    protected:
        void Invoke(const PluginsReloadReq& req, WriteCb<PluginsReloadRes> cb) override;

    private:
        porla::Lua::PluginEngine& m_plugin_engine;
    };
}
