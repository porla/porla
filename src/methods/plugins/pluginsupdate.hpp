#pragma once

#include "../method.hpp"
#include "pluginsupdate_reqres.hpp"

namespace porla::Lua
{
    class PluginEngine;
}

namespace porla::Methods
{
    struct PluginsUpdateOptions
    {
        porla::Lua::PluginEngine& plugin_engine;
    };

    class PluginsUpdate : public Method<PluginsUpdateReq, PluginsUpdateRes>
    {
    public:
        explicit PluginsUpdate(const PluginsUpdateOptions& options);

    protected:
        void Invoke(const PluginsUpdateReq& req, WriteCb<PluginsUpdateRes> cb) override;

    private:
        PluginsUpdateOptions m_options;
    };
}
