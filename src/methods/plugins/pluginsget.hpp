#pragma once

#include "../method.hpp"
#include "pluginsget_reqres.hpp"

namespace porla::Lua
{
    class PluginEngine;
}

namespace porla::Methods
{
    template <bool SSL> class PluginsGet : public Method<PluginsGetReq, PluginsGetRes, SSL>
    {
    public:
        explicit PluginsGet(porla::Lua::PluginEngine& plugin_engine);

    protected:
        void Invoke(const PluginsGetReq& req, WriteCb<PluginsGetRes, SSL> cb) override;

    private:
        porla::Lua::PluginEngine& m_plugin_engine;
    };
}
