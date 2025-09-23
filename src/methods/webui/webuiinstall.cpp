#include "webuiinstall.hpp"

#include <fstream>

#include <boost/log/trivial.hpp>

#include "../../utils/base64.hpp"

namespace fs = std::filesystem;

using porla::Methods::WebUI::WebUIInstall;
using porla::Methods::WebUI::WebUIInstallReq;
using porla::Methods::WebUI::WebUIInstallRes;
using porla::Utils::Base64;

WebUIInstall::WebUIInstall(const porla::Config& cfg, boost::signals2::signal<void(const char*, size_t)>& installed_signal)
    : m_cfg(cfg)
    , m_installed_signal(installed_signal)
{
}

void WebUIInstall::Invoke(const WebUIInstallReq& req, WriteCb<WebUIInstallRes> cb)
{
    if (!m_cfg.http_webui_enabled.value_or(true))
    {
        return cb.Error(-1, "Web UI not enabled");
    }

    const auto buffer = Base64::Decode(req.data);
    const auto webui_file = m_cfg.state_dir.value_or(fs::current_path()) / m_cfg.http_webui_file.value_or("webui.zip");

    {
        BOOST_LOG_TRIVIAL(info) << "Installing new web UI (" << buffer.size() << " bytes)";

        std::ofstream webui_file_stream(webui_file, std::ios::binary);
        webui_file_stream << buffer;
        webui_file_stream.flush();
        webui_file_stream.close();
    }

    m_installed_signal(buffer.c_str(), buffer.size());

    cb.Ok(WebUIInstallRes{});
}
