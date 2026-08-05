#pragma once

#include <filesystem>

#include <sqlite3.h>

#include "../../config.hpp"
#include "../method.hpp"
#include "pluginsadd_reqres.hpp"

namespace porla::Lua
{
    class PluginEngine;
}

namespace porla::Methods
{
    class PluginsAdd : public Method<PluginsAddReq, PluginsAddRes>
    {
    public:
        explicit PluginsAdd(sqlite3* db, porla::Lua::PluginEngine& plugins);

    protected:
        void Invoke(const PluginsAddReq& req, WriteCb<PluginsAddRes> cb) override;

    private:
        sqlite3* m_db;
        porla::Lua::PluginEngine& m_plugins;
    };
}
