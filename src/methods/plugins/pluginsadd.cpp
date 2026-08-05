#include "pluginsadd.hpp"

#include <boost/log/trivial.hpp>

#include "../../data/models/plugins.hpp"
#include "../../lua/plugin.hpp"
#include "../../lua/pluginengine.hpp"
#include "../../utils/base64.hpp"

using porla::Data::Models::Plugins;
using porla::Lua::PluginEngine;
using porla::Methods::PluginsAdd;
using porla::Methods::PluginsAddReq;
using porla::Methods::PluginsAddRes;
using porla::Utils::Base64;

PluginsAdd::PluginsAdd(sqlite3* db, porla::Lua::PluginEngine& plugins)
    : m_db(db)
    , m_plugins(plugins)
{
}

void PluginsAdd::Invoke(const PluginsAddReq& req, WriteCb<PluginsAddRes> cb)
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

        const auto plugin_id = Plugins::Insert(
            m_db,
            Plugins::Plugin{
                .id       = -1,
                .type     = req.type,
                .data     = std::vector<char>(req.data.begin(), req.data.end()),
                .config   = req.config,
                .metadata = req.metadata.value_or({})
            });

        BOOST_LOG_TRIVIAL(info) << "Plugin " << plugin_id << " installed with path " << plugin_path;

        m_plugins.Load(plugin_id);

        return cb.Ok(PluginsAddRes{
            .id = plugin_id
        });
    }

    if (req.type == "archive")
    {
        const auto archive_buffer = Base64::Decode(req.data);

        try
        {
            return cb.Ok(PluginsAddRes{
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
        catch(const std::exception& e)
        {
            std::stringstream ss;
            ss << "Failed to install plugin: ";
            ss << e.what();

            return cb.Error(-201, ss.str());
        }
    }

    return cb.Error(-1, "Invalid plugin type");
}
