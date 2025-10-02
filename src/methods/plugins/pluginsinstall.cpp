#include "pluginsinstall.hpp"

#include <boost/log/trivial.hpp>

#include "../../lua/plugin.hpp"
#include "../../lua/pluginengine.hpp"
#include "../../utils/base64.hpp"

using porla::Lua::PluginEngine;

using porla::Methods::PluginsInstall;
using porla::Methods::PluginsInstallReq;
using porla::Methods::PluginsInstallRes;
using porla::Utils::Base64;

PluginsInstall::PluginsInstall(porla::Lua::PluginEngine& plugins)
    : m_plugins(plugins)
{
}

void PluginsInstall::Invoke(const PluginsInstallReq& req, WriteCb<PluginsInstallRes> cb)
{
    if (req.type == "path")
    {
        fs::path plugin_path = req.data;

        if (!plugin_path.is_absolute())
        {
            return cb.Error(-101, "Plugin path must be absolute");
        }

        if (!fs::exists(req.data))
        {
            return cb.Error(-102, "Plugin path does not exist");
        }

        return cb.Ok(PluginsInstallRes{
            .id = m_plugins.InstallFromPath(
                plugin_path,
                req.config,
                req.metadata.has_value()
                    ? json(req.metadata.value())
                    : json())
        });
    }

    if (req.type == "archive")
    {
        const auto archive_buffer = Base64::Decode(req.data);

        return cb.Ok(PluginsInstallRes{
            .id = m_plugins.InstallFromArchive(
                std::vector<char>(
                    archive_buffer.begin(),
                    archive_buffer.end()),
                req.config,
                req.metadata.has_value()
                    ? json(req.metadata.value())
                    : json())
        });
    }

    return cb.Error(-1, "Invalid plugin type");
}
