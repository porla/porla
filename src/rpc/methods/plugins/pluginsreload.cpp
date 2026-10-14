#include "pluginsreload.hpp"

#include "../../../plugins.hpp"

using porla::Rpc::Methods::Plugins::PluginsReload;
using porla::Rpc::Methods::Plugins::PluginsReloadReq;

PluginsReload::PluginsReload(porla::Plugins& plugins)
    : m_plugins(plugins)
{
}

void PluginsReload::Execute(const PluginsReloadReq& req, ResponseWriterHandle cb)
{
    m_plugins.Reload(req.id);
    cb->Ok();
}
