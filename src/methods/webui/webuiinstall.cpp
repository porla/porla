#include "webuiinstall.hpp"

#include <boost/log/trivial.hpp>

#include "../../buildinfo.hpp"
#include "../../webui.hpp"

using porla::Methods::WebUI::WebUIInstall;
using porla::Methods::WebUI::WebUIInstallReq;
using porla::Methods::WebUI::WebUIInstallRes;

WebUIInstall::WebUIInstall(const std::weak_ptr<porla::WebUI>& webui)
    : m_webui(webui)
{
}

void WebUIInstall::Invoke(const WebUIInstallReq& req, WriteCb<WebUIInstallRes> cb)
{
    auto webui = m_webui.lock();

    if (!webui)
    {
        return cb.Error(-1, "Cannot lock curl");
    }

    auto callback = std::make_shared<WriteCb<WebUIInstallRes>>(std::move(cb));

    webui->Install("tags/" + req.version, [callback]()
    {
        callback->Ok({});
    });
}
