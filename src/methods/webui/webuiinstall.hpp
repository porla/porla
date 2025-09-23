#pragma once

#include <boost/signals2.hpp>

#include "../../config.hpp"
#include "../method.hpp"
#include "webuiinstall_reqres.hpp"

namespace porla::Methods::WebUI
{
    class WebUIInstall : public Method<WebUIInstallReq, WebUIInstallRes>
    {
    public:
        explicit WebUIInstall(const porla::Config& cfg, boost::signals2::signal<void(const char*, size_t)>& installed_signal);

    protected:
        void Invoke(const WebUIInstallReq& req, WriteCb<WebUIInstallRes> cb) override;

    private:
        const porla::Config& m_cfg;
        boost::signals2::signal<void(const char*, size_t)>& m_installed_signal;
    };
}
