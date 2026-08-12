#pragma once

#include "../method.hpp"
#include "webuiinstall_reqres.hpp"

namespace porla
{
    class WebUI;
}

namespace porla::Methods::WebUI
{
    class WebUIInstall : public Method<WebUIInstallReq, WebUIInstallRes>
    {
    public:
        explicit WebUIInstall(const std::weak_ptr<porla::WebUI>& cm);

    protected:
        void Invoke(const WebUIInstallReq& req, WriteCb<WebUIInstallRes> cb) override;

    private:
        std::weak_ptr<porla::WebUI> m_webui;
    };
}
