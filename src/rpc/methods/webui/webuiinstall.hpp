#pragma once

#include "../../typedmethod.hpp"
#include "webuiinstall_reqres.hpp"

namespace porla
{
    class WebUI;
}

namespace porla::Rpc::Methods::WebUI
{
    class WebUIInstall : public TypedMethod<WebUIInstallReq, WebUIInstallRes>
    {
    public:
        explicit WebUIInstall(const std::weak_ptr<porla::WebUI>& cm);

    protected:
        void Execute(const WebUIInstallReq& req, ResponseWriterHandle cb) override;

    private:
        std::weak_ptr<porla::WebUI> m_webui;
    };
}
