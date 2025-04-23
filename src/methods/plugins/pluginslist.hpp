#pragma once

#include "../method.hpp"
#include "pluginslist_reqres.hpp"

namespace porla::Lua
{
    class PluginEngine;
}

namespace porla::Methods
{
    template <bool SSL> class PluginsList : public Method<PluginsListReq, PluginsListRes, SSL>
    {
    public:
        explicit PluginsList(porla::Lua::PluginEngine& plugin_engine);

    protected:
        void Invoke(const PluginsListReq& req, WriteCb<PluginsListRes, SSL> cb) override;

    private:
        porla::Lua::PluginEngine& m_plugin_engine;
    };
}
