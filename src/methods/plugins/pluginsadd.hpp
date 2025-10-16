#pragma once

#include <filesystem>

#include "../../config.hpp"
#include "../method.hpp"
#include "pluginsadd_reqres.hpp"

namespace porla::Lua
{
    class PluginEngine;
}

namespace porla::Methods
{
    class PluginsAdd : public Method<PluginsAddReq, PluginsAddRes>
    {
    public:
        explicit PluginsAdd(porla::Lua::PluginEngine& plugins);

    protected:
        void Invoke(const PluginsAddReq& req, WriteCb<PluginsAddRes> cb) override;

    private:
        porla::Lua::PluginEngine& m_plugins;
    };
}
