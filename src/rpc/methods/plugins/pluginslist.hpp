#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"

#include "pluginslist_reqres.hpp"

namespace porla::Lua
{
    class PluginEngine;
}

namespace porla::Rpc::Methods::Plugins
{
    class PluginsList : public TypedMethod<PluginsListReq, PluginsListRes>
    {
    public:
        explicit PluginsList(sqlite3* db, porla::Lua::PluginEngine& plugin_engine);

    protected:
        void Execute(const PluginsListReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        porla::Lua::PluginEngine& m_plugin_engine;
    };
}
