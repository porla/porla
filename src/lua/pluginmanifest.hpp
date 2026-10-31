#pragma once

#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "../json/utils.hpp"

namespace porla::Lua
{
    struct PluginSource;

    struct PluginManifest
    {
        struct Ui
        {
            std::string                             entry;
            std::optional<std::vector<std::string>> rpc;
        };

        std::optional<std::string> name;
        std::optional<std::string> slug;
        std::optional<std::string> version;
        std::optional<Ui>          ui;

        // nullopt when the source has no plugin.json. throws "Invalid plugin.json: ..." when it
        // isn't valid JSON, a known key has the wrong type, ui.entry isn't a file in the ui/
        // folder or ui.rpc lists an x.* method
        static std::optional<PluginManifest> FromSource(const PluginSource& source);
    };

    PORLA_JSON_REQUEST(PluginManifest::Ui, entry, rpc)
    PORLA_JSON_REQUEST(PluginManifest, name, slug, version, ui)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(PluginManifest::Ui, entry, rpc)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(PluginManifest, name, slug, version, ui)
}
