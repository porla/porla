#pragma once

#include <sqlite3.h>

#include "../method.hpp"
#include "pluginsremove_reqres.hpp"

namespace porla::Lua
{
    class PluginEngine;
}

namespace porla::Methods
{
    class PluginsRemove : public Method<PluginsRemoveReq, PluginsRemoveRes>
    {
    public:
        explicit PluginsRemove(sqlite3* db, porla::Lua::PluginEngine& plugin_engine);

    protected:
        void Invoke(const PluginsRemoveReq& req, WriteCb<PluginsRemoveRes> cb) override;

    private:
        sqlite3* m_db;
        porla::Lua::PluginEngine& m_plugin_engine;
    };
}
