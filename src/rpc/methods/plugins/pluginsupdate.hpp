#pragma once

#include "../../typedmethod.hpp"

#include "pluginsupdate_reqres.hpp"

namespace porla
{
    class Plugins;
}

namespace porla::Rpc::Methods::Plugins
{
    class PluginsUpdate : public TypedMethod<PluginsUpdateReq, PluginsUpdateRes>
    {
    public:
        explicit PluginsUpdate(porla::Plugins& plugins);

    protected:
        bool CanInvoke(const porla::Auth::Context& auth_ctx) override
        {
            return auth_ctx.IsAuthenticated() && auth_ctx.kind == porla::Auth::Context::Kind::User;
        }

        void Execute(const PluginsUpdateReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Plugins& m_plugins;
    };
}
