#include "webuiinstall.hpp"

#include <boost/log/trivial.hpp>

#include "../../../buildinfo.hpp"
#include "../../../webui.hpp"

using porla::Rpc::Methods::WebUI::WebUIInstall;
using porla::Rpc::Methods::WebUI::WebUIInstallReq;
using porla::Rpc::Methods::WebUI::WebUIInstallRes;

WebUIInstall::WebUIInstall(const std::weak_ptr<porla::WebUI>& webui)
    : m_webui(webui)
{
}

void WebUIInstall::Execute(const WebUIInstallReq& req, ResponseWriterHandle cb)
{
    auto webui = m_webui.lock();

    if (!webui)
    {
        return cb->Error(-1, "Cannot lock curl");
    }

    webui->Install("tags/" + req.version, [cb]()
    {
        cb->Ok({});
    });
}
