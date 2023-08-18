#pragma once

#include "../method.hpp"
#include "pluginslist_reqres.hpp"

namespace porla::Lua
{
    class PluginEngine;
}

namespace porla::Methods
{
    class PluginsList : public Method<PluginsListReq, PluginsListRes>
    {
    public:
        explicit PluginsList(porla::Lua::PluginEngine& plugin_engine);

    protected:
        void Invoke(const PluginsListReq& req, WriteCb<PluginsListRes> cb) override;

    private:
        porla::Lua::PluginEngine& m_plugin_engine;
    };
}
