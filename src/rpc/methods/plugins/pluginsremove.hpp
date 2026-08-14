#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"

#include "pluginsremove_reqres.hpp"

namespace porla::Lua
{
    class PluginEngine;
}

namespace porla::Rpc::Methods::Plugins
{
    class PluginsRemove : public TypedMethod<PluginsRemoveReq, PluginsRemoveRes>
    {
    public:
        explicit PluginsRemove(sqlite3* db, porla::Lua::PluginEngine& plugin_engine);

    protected:
        void Execute(const PluginsRemoveReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        porla::Lua::PluginEngine& m_plugin_engine;
    };
}
