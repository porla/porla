#pragma once

#include "../../typedmethod.hpp"

#include "pluginslist_reqres.hpp"

namespace porla
{
    class Plugins;
}

namespace porla::Rpc::Methods::Plugins
{
    class PluginsList : public TypedMethod<PluginsListReq, PluginsListRes>
    {
    public:
        explicit PluginsList(porla::Plugins& plugins);

    protected:
        bool CanInvoke(const porla::Auth::Context& auth_ctx) override
        {
            return auth_ctx.IsAuthenticated() && auth_ctx.kind == porla::Auth::Context::Kind::User;
        }

        void Execute(const PluginsListReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Plugins& m_plugins;
    };
}
