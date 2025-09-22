#include "pluginsupdate.hpp"

#include <boost/log/trivial.hpp>

#include "../../lua/plugin.hpp"
#include "../../lua/pluginengine.hpp"

using porla::Lua::PluginEngine;
using porla::Lua::PluginInstallOptions;

using porla::Methods::PluginsUpdate;
using porla::Methods::PluginsUpdateReq;
using porla::Methods::PluginsUpdateRes;

PluginsUpdate::PluginsUpdate(const PluginsUpdateOptions& options)
    : m_options(options)
{
}

void PluginsUpdate::Invoke(const PluginsUpdateReq& req, WriteCb<PluginsUpdateRes> cb)
{
    return cb.Error(-2, "Updating plugins has been removed");
}
