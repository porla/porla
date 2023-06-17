#pragma once

#include "../method.hpp"
#include "pluginsconfigure_reqres.hpp"

namespace porla::Lua::Plugins
{
    class PluginEngine;
}

namespace porla::Methods
{
    class PluginsConfigure : public Method<PluginsConfigureReq, PluginsConfigureRes>
    {
    public:
        explicit PluginsConfigure(porla::Lua::Plugins::PluginEngine& plugin_engine);

    protected:
        void Invoke(const PluginsConfigureReq& req, WriteCb<PluginsConfigureRes> cb) override;

    private:
        porla::Lua::Plugins::PluginEngine& m_plugin_engine;
    };
}
