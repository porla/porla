#pragma once

#include <sqlite3.h>

#include "../method.hpp"
#include "pluginsupdate_reqres.hpp"

namespace porla::Lua
{
    class PluginEngine;
}

namespace porla::Methods
{
    class PluginsUpdate : public Method<PluginsUpdateReq, PluginsUpdateRes>
    {
    public:
        explicit PluginsUpdate(sqlite3* db, porla::Lua::PluginEngine& plugin_engine);

    protected:
        void Invoke(const PluginsUpdateReq& req, WriteCb<PluginsUpdateRes> cb) override;

    private:
        sqlite3* m_db;
        porla::Lua::PluginEngine& m_plugin_engine;
    };
}
