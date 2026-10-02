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
        bool CanInvoke(const porla::Auth::Context& auth_ctx) override
        {
            return auth_ctx.IsAuthenticated() && auth_ctx.kind == porla::Auth::Context::Kind::User;
        }

        void Execute(const PluginsAddReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        porla::Lua::PluginEngine& m_plugins;
    };
}
