#pragma once

#include "../method.hpp"
#include "pluginsupdate_reqres.hpp"

namespace porla::Lua
{
    class PluginEngine;
}

namespace porla::Methods
{
    class PluginsUpdate : public Method<PluginsUpdateReq, PluginsUpdateRes>
    {
    public:
        explicit PluginsUpdate(porla::Lua::PluginEngine& plugin_engine);

    protected:
        void Invoke(const PluginsUpdateReq& req, WriteCb<PluginsUpdateRes> cb) override;

    private:
        porla::Lua::PluginEngine& m_plugin_engine;
    };
}
