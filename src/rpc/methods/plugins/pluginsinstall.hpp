#pragma once

#include <filesystem>
#include <memory>

#include <sqlite3.h>

#include "../../typedmethod.hpp"

#include "pluginsinstall_reqres.hpp"

namespace porla
{
    class CurlMulti;
}

namespace porla::Lua
{
    class PluginEngine;
}

namespace porla::Rpc::Methods::Plugins
{
    class PluginsInstall : public TypedMethod<PluginsInstallReq, PluginsInstallRes>
    {
    public:
        explicit PluginsInstall(sqlite3* db, std::weak_ptr<CurlMulti> cm, porla::Lua::PluginEngine& plugin_engine);

    protected:
        void Execute(const PluginsInstallReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        std::weak_ptr<CurlMulti> m_cm;
        porla::Lua::PluginEngine& m_plugin_engine;
    };
}
