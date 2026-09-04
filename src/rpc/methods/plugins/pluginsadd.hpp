#pragma once

#include <filesystem>

#include <sqlite3.h>

#include "../../typedmethod.hpp"

#include "pluginsadd_reqres.hpp"

namespace porla::Lua
{
    class PluginEngine;
}

namespace porla::Rpc::Methods::Plugins
{
    class PluginsAdd : public TypedMethod<PluginsAddReq, PluginsAddRes>
    {
    public:
        explicit PluginsAdd(sqlite3* db, porla::Lua::PluginEngine& plugins);

    protected:
        void Execute(const PluginsAddReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        porla::Lua::PluginEngine& m_plugins;
    };
}
