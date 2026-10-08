#pragma once

#include "../../typedmethod.hpp"

#include "pluginsremove_reqres.hpp"

namespace porla
{
    class Plugins;
}

namespace porla::Rpc::Methods::Plugins
{
    class PluginsRemove : public TypedMethod<PluginsRemoveReq, PluginsRemoveRes>
    {
    public:
        explicit PluginsRemove(porla::Plugins& plugins);

    protected:
        bool CanInvoke(const porla::Auth::Context& auth_ctx) override
        {
            return auth_ctx.IsAuthenticated() && auth_ctx.kind == porla::Auth::Context::Kind::User;
        }

        void Execute(const PluginsRemoveReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Plugins& m_plugins;
    };
}
