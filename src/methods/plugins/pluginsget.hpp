#pragma once

#include <sqlite3.h>

#include "../method.hpp"
#include "pluginsget_reqres.hpp"

namespace porla::Lua
{
    class PluginEngine;
}

namespace porla::Methods
{
    class PluginsGet : public Method<PluginsGetReq, PluginsGetRes>
    {
    public:
        explicit PluginsGet(sqlite3* db, porla::Lua::PluginEngine& plugin_engine);

    protected:
        void Invoke(const PluginsGetReq& req, WriteCb<PluginsGetRes> cb) override;

    private:
        sqlite3* m_db;
        porla::Lua::PluginEngine& m_plugin_engine;
    };
}
