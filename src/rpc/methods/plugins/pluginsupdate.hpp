#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"

#include "pluginsupdate_reqres.hpp"

namespace porla::Lua
{
    class PluginEngine;
}

namespace porla::Rpc::Methods::Plugins
{
    class PluginsUpdate : public TypedMethod<PluginsUpdateReq, PluginsUpdateRes>
    {
    public:
        explicit PluginsUpdate(sqlite3* db, porla::Lua::PluginEngine& plugin_engine);

    protected:
        void Execute(const PluginsUpdateReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        porla::Lua::PluginEngine& m_plugin_engine;
    };
}
