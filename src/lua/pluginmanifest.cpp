#include "pluginmanifest.hpp"

#include <stdexcept>

#include "pluginsource.hpp"

using porla::Lua::PluginManifest;

std::optional<PluginManifest> PluginManifest::FromSource(const PluginSource& source)
{
    const auto file = source.sources.find("plugin.json");

    if (file == source.sources.end())
    {
        return std::nullopt;
    }

    try
    {
        auto manifest = porla::Json::Parse(std::string_view(file->second.data(), file->second.size())).get<PluginManifest>();

        if (manifest.ui.has_value())
        {
            if (!manifest.ui->entry.has_value() && !manifest.ui->settings.has_value())
            {
                throw std::invalid_argument("ui needs an entry or a settings page");
            }

            // exact: a path leaving ui/ or starting with / is never a key in the plugin's sources
            for (const auto& [key, page] : { std::pair{ "entry", manifest.ui->entry }, std::pair{ "settings", manifest.ui->settings } })
            {
                if (page.has_value() && !source.sources.contains("ui/" + *page))
                {
                    throw std::invalid_argument(std::string("ui.") + key + " must be a file in the plugin's ui/ folder");
                }
            }

            // the plugin's own x.<slug>.* need no listing, and other plugins' aren't allowed
            for (const auto& method : manifest.ui->rpc.value_or(std::vector<std::string>{}))
            {
                if (method.starts_with("x."))
                {
                    throw std::invalid_argument("ui.rpc can only list core methods, not '" + method + "'");
                }
            }
        }

        return manifest;
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("Invalid plugin.json: ") + e.what());
    }
}
