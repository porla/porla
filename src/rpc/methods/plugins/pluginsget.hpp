#pragma once

#include "../../typedmethod.hpp"

#include "pluginsget_reqres.hpp"

namespace porla
{
    class Plugins;
}

namespace porla::Rpc::Methods::Plugins
{
    class PluginsGet : public TypedMethod<PluginsGetReq, PluginsGetRes>
    {
    public:
        explicit PluginsGet(porla::Plugins& plugins);

    protected:
        bool CanInvoke(const porla::Auth::Context& auth_ctx) override
        {
            return auth_ctx.IsAuthenticated() && auth_ctx.kind == porla::Auth::Context::Kind::User;
        }

        void Execute(const PluginsGetReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Plugins& m_plugins;
    };
}
