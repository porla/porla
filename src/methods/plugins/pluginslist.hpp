#pragma once

#include <sqlite3.h>

#include "../method.hpp"
#include "pluginslist_reqres.hpp"

namespace porla::Lua
{
    class PluginEngine;
}

namespace porla::Methods
{
    class PluginsList : public Method<PluginsListReq, PluginsListRes>
    {
    public:
        explicit PluginsList(sqlite3* db, porla::Lua::PluginEngine& plugin_engine);

    protected:
        void Invoke(const PluginsListReq& req, WriteCb<PluginsListRes> cb) override;

    private:
        sqlite3* m_db;
        porla::Lua::PluginEngine& m_plugin_engine;
    };
}
