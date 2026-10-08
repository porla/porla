#include "pluginsremove.hpp"

#include "../../../plugins.hpp"

using porla::Rpc::Methods::Plugins::PluginsRemove;
using porla::Rpc::Methods::Plugins::PluginsRemoveReq;
using porla::Rpc::Methods::Plugins::PluginsRemoveRes;

PluginsRemove::PluginsRemove(porla::Plugins& plugins)
    : m_plugins(plugins)
{
}

void PluginsRemove::Execute(const PluginsRemoveReq& req, ResponseWriterHandle cb)
{
    const auto plugin = m_plugins.Get(req.id);

    if (!plugin.has_value())
    {
        return cb->Error(-1, "Plugin not found");
    }

    m_plugins.Remove(req.id);

    cb->Ok({});
}
