#pragma once

#include <filesystem>

#include "../../config.hpp"
#include "../method.hpp"
#include "pluginsinstall_reqres.hpp"

namespace porla::Lua
{
    class PluginEngine;
}

namespace porla::Methods
{
    class PluginsInstall : public Method<PluginsInstallReq, PluginsInstallRes>
    {
    public:
        explicit PluginsInstall(porla::Lua::PluginEngine& plugins);

    protected:
        void Invoke(const PluginsInstallReq& req, WriteCb<PluginsInstallRes> cb) override;

    private:
        porla::Lua::PluginEngine& m_plugins;
    };
}
