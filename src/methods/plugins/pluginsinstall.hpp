#pragma once

#include <memory>

#include <sqlite3.h>

#include "../method.hpp"
#include "pluginsinstall_reqres.hpp"

namespace porla
{
    class CurlMulti;
}

namespace porla::Lua
{
    class PluginEngine;
}

namespace porla::Methods::Plugins
{
    class PluginsInstall : public Method<PluginsInstallReq, PluginsInstallRes>
    {
    public:
        explicit PluginsInstall(sqlite3* db, std::weak_ptr<CurlMulti> cm, porla::Lua::PluginEngine& plugin_engine);

    protected:
        void Invoke(const PluginsInstallReq& req, WriteCb<PluginsInstallRes> cb) override;

    private:
        sqlite3* m_db;
        std::weak_ptr<CurlMulti> m_cm;
        porla::Lua::PluginEngine& m_plugin_engine;
    };
}
