#pragma once

#include <memory>

#include <sqlite3.h>

#include "../../typedasyncmethod.hpp"

#include "pluginsupgrade_reqres.hpp"

namespace porla
{
    class CurlMulti;
    class Plugins;
}

namespace porla::Rpc::Methods::Plugins
{
    class PluginsUpgrade : public TypedAsyncMethod<PluginsUpgradeReq, PluginsUpgradeRes>
    {
    public:
        explicit PluginsUpgrade(
            boost::asio::io_context& io,
            sqlite3* db,
            std::weak_ptr<CurlMulti> cm,
            porla::Plugins& plugins,
            const std::filesystem::path& state_dir);

    protected:
        bool CanInvoke(const porla::Auth::Context& auth_ctx) override
        {
            return auth_ctx.IsAuthenticated() && auth_ctx.kind == porla::Auth::Context::Kind::User;
        }

        boost::asio::awaitable<void> ExecuteAsync(PluginsUpgradeReq req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        std::weak_ptr<CurlMulti> m_cm;
        porla::Plugins& m_plugins;
        std::filesystem::path m_state_dir;
    };
}
