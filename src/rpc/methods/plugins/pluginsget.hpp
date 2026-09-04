#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"

#include "pluginsget_reqres.hpp"

namespace porla::Lua
{
    class PluginEngine;
}

namespace porla::Rpc::Methods::Plugins
{
    class PluginsGet : public TypedMethod<PluginsGetReq, PluginsGetRes>
    {
    public:
        explicit PluginsGet(sqlite3* db, porla::Lua::PluginEngine& plugin_engine);

    protected:
        void Execute(const PluginsGetReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        porla::Lua::PluginEngine& m_plugin_engine;
    };
}
