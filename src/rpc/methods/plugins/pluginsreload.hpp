#pragma once

#include "../../typedmethod.hpp"

#include "pluginsreload_reqres.hpp"

namespace porla::Lua
{
    class PluginEngine;
}

namespace porla::Rpc::Methods::Plugins
{
    class PluginsReload : public TypedMethod<PluginsReloadReq, PluginsReloadRes>
    {
    public:
        explicit PluginsReload(porla::Lua::PluginEngine& plugin_engine);

    protected:
        void Execute(const PluginsReloadReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Lua::PluginEngine& m_plugin_engine;
    };
}
