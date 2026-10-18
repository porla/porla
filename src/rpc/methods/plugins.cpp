#include "plugins.hpp"

#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>

#include <boost/asio/awaitable.hpp>
#include <toml++/toml.hpp>

#include "../functionmethod.hpp"
#include "../jsonrpc.hpp"

#include "../../curlmulti.hpp"
#include "../../json/github.hpp"
#include "../../json/utils.hpp"
#include "../../lua/plugin.hpp"
#include "../../lua/pluginsource.hpp"
#include "../../plugins.hpp"

using boost::asio::awaitable;

namespace fs = std::filesystem;

namespace porla::Rpc::Methods::Plugins
{
    namespace
    {
        struct AddReq
        {
            std::string                path;
            std::optional<std::string> config;
        };

        struct GetReq
        {
            int id;
        };

        struct GetRes
        {
            struct Plugin
            {
                int                                   id;
                std::string                           path;
                std::optional<std::string>            name;
                std::optional<std::string>            version;
                std::optional<std::string>            config;
                std::map<std::string, nlohmann::json> metadata;
                bool                                  is_loaded;
            };

            Plugin plugin;
        };

        struct InstallReq
        {
            std::string                owner;
            std::string                repository;
            std::string                version;
            std::optional<std::string> config;
        };

        struct ListReq
        {
        };

        struct ListRes
        {
            struct Plugin
            {
                int                                   id;
                std::string                           path;
                std::optional<std::string>            name;
                std::optional<std::string>            version;
                std::map<std::string, nlohmann::json> metadata;
                bool                                  is_loaded;
            };

            std::vector<Plugin> plugins;
        };

        struct ReloadReq
        {
            int id;
        };

        struct RemoveReq
        {
            int id;
        };

        struct UpdateReq
        {
            int                        id;
            std::string                path;
            std::optional<std::string> config;
        };

        struct UpgradeReq
        {
            int         id;
            std::string version;
        };

        PORLA_JSON_REQUEST(AddReq, path, config)
        PORLA_JSON_REQUEST(GetReq, id)
        PORLA_JSON_REQUEST(InstallReq, owner, repository, version, config)
        PORLA_JSON_NO_PARAMS(ListReq)
        PORLA_JSON_REQUEST(ReloadReq, id)
        PORLA_JSON_REQUEST(RemoveReq, id)
        PORLA_JSON_REQUEST(UpdateReq, id, path, config)
        PORLA_JSON_REQUEST(UpgradeReq, id, version)

        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
            GetRes::Plugin,
            id,
            path,
            name,
            version,
            config,
            metadata,
            is_loaded)

        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(GetRes, plugin)

        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
            ListRes::Plugin,
            id,
            path,
            name,
            version,
            metadata,
            is_loaded)

        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(ListRes, plugins)

        awaitable<nlohmann::json> Add(porla::Plugins& plugins, const AddReq& req)
        {
            fs::path plugin_path = req.path;

            if (!plugin_path.is_absolute())
            {
                throw Error(-2, "Plugin path must be absolute");
            }

            if (!fs::exists(plugin_path))
            {
                throw Error(-3, "Plugin path does not exist");
            }

            const auto plugin_id = plugins.Add(porla::Plugins::Plugin{
                .id       = -1,
                .path     = plugin_path,
                .config   = req.config,
                .metadata = {}
            });

            co_return nlohmann::json({
                {"id", plugin_id}
            });
        }

        awaitable<GetRes> Get(porla::Plugins& plugins, const GetReq& req)
        {
            const auto plugin = plugins.Get(req.id);

            if (!plugin.has_value())
            {
                throw Error(-1, "Plugin not found");
            }

            const auto instance = plugins.Instance(req.id);

            auto name     = std::optional<std::string>();
            auto version = std::optional<std::string>();

            if (instance != nullptr)
            {
                const auto& source      = instance->Source();
                const auto& plugin_toml = source.sources.find("plugin.toml");

                if (plugin_toml != source.sources.end())
                {
                    const auto parsed_toml = toml::parse(plugin_toml->second.data());
                    if (const auto parsed_name    = parsed_toml["name"].value<std::string>())    name    = parsed_name.value();
                    if (const auto parsed_version = parsed_toml["version"].value<std::string>()) version = parsed_version.value();
                }
            }

            co_return GetRes{
                .plugin = GetRes::Plugin{
                    .id        = plugin->id,
                    .path      = plugin->path,
                    .name      = name,
                    .version   = version,
                    .config    = plugin->config,
                    .metadata  = plugin->metadata,
                    .is_loaded = instance != nullptr
                }
            };
        }

        awaitable<nlohmann::json> Install(porla::Plugins& plugins, CurlMulti& curl, const fs::path& state_dir, const InstallReq& req)
        {
            std::stringstream url;
            url << "https://api.github.com/repos/" << req.owner << "/" << req.repository << "/releases/tags/" << req.version;

            BOOST_LOG_TRIVIAL(trace) << "Begin installation of plugin from " << url.str();

            const auto [
                release_status,
                release_body ] = co_await curl.AsyncHttpGet(url.str(), boost::asio::use_awaitable);

            if (release_status != 200)
            {
                throw Error(-2, "Release not found - HTTP status: " + std::to_string(release_status));
            }

            porla::Json::GitHubRelease release;

            try
            {
                release = nlohmann::json::parse(release_body).get<porla::Json::GitHubRelease>();
            }
            catch (const std::exception& e)
            {
                throw Error(-3, "Failed to parse release body as JSON", {{"what", e.what()}});
            }

            if (release.assets.size() == 0)
            {
                throw Error(-4, "Release has no downloadable asset");
            }

            BOOST_LOG_TRIVIAL(info)
                << "Found version "
                << release.tag_name
                << " of plugin - fetching from " << release.assets[0].browser_download_url;

            const auto [
                asset_status,
                asset_body ] = co_await curl.AsyncHttpGet(release.assets[0].browser_download_url, boost::asio::use_awaitable);

            if (asset_status != 200)
            {
                throw Error(-5, "Failed to fetch release asset");
            }

            std::stringstream zip_file_name;
            zip_file_name << req.owner << "_" << req.repository << "_" << release.tag_name << ".zip";

            const auto plugins_dir = fs::absolute(state_dir / "plugins").lexically_normal();
            const auto plugin_zip  = plugins_dir / zip_file_name.str();

            if (!fs::exists(plugins_dir))
            {
                fs::create_directories(plugins_dir);
            }

            {
                std::ofstream out(plugin_zip, std::ios::binary);

                out.write(
                    asset_body.data(),
                    asset_body.size());

                if (!out)
                {
                    throw Error(-6, "Failed to write plugin archive to " + plugin_zip.string());
                }

                BOOST_LOG_TRIVIAL(debug) << "Wrote plugin to " << plugin_zip;
            }

            const auto plugin_id = plugins.Add(porla::Plugins::Plugin{
                .id       = -1,
                .path     = plugin_zip,
                .config   = req.config,
                .metadata = {
                    {"source", "github"},
                    {"owner", req.owner},
                    {"repository", req.repository},
                    {"version", release.tag_name}
                }
            });

            co_return nlohmann::json({
                {"id", plugin_id}
            });
        }

        awaitable<ListRes> List(porla::Plugins& plugins, const ListReq& req)
        {
            ListRes res = {};

            for (const auto& plugin : plugins.All())
            {
                const auto instance    = plugins.Instance(plugin.id);
                const auto plugin_path = fs::path(plugin.path);

                auto name    = std::optional<std::string>(plugin_path.filename());
                auto version = std::optional<std::string>();

                if (instance != nullptr)
                {
                    const auto& source      = instance->Source();
                    const auto& plugin_toml = source.sources.find("plugin.toml");

                    if (plugin_toml != source.sources.end())
                    {
                        const auto parsed_toml = toml::parse(plugin_toml->second.data());
                        if (const auto parsed_name    = parsed_toml["name"].value<std::string>())    name    = parsed_name.value();
                        if (const auto parsed_version = parsed_toml["version"].value<std::string>()) version = parsed_version.value();
                    }
                }

                res.plugins.emplace_back(ListRes::Plugin{
                    .id        = plugin.id,
                    .path      = plugin.path,
                    .name      = name,
                    .version   = version,
                    .metadata  = plugin.metadata,
                    .is_loaded = instance != nullptr
                });
            }

            co_return res;
        }

        awaitable<void> Reload(porla::Plugins& plugins, const ReloadReq& req)
        {
            plugins.Reload(req.id);
            co_return;
        }


        awaitable<void> Remove(porla::Plugins& plugins, const RemoveReq& req)
        {
            const auto plugin = plugins.Get(req.id);

            if (!plugin.has_value())
            {
                throw Error(-1, "Plugin not found");
            }

            plugins.Remove(req.id);

            co_return;
        }

        awaitable<void> Update(porla::Plugins& plugins, const UpdateReq& req)
        {
            auto plugin = plugins.Get(req.id);

            if (!plugin.has_value())
            {
                throw Error(-1, "Plugin not found");
            }

            const fs::path plugin_path = req.path;

            if (!plugin_path.is_absolute())
            {
                throw Error(-2, "Plugin path must be absolute");
            }

            if (!fs::exists(plugin_path))
            {
                throw Error(-3, "Plugin path does not exist");
            }

            plugin->config   = req.config;
            plugin->path     = plugin_path.lexically_normal();

            plugins.Update(*plugin);

            co_return;
        }

        awaitable<void> Upgrade(porla::Plugins& plugins, CurlMulti& curl, const fs::path& state_dir, const UpgradeReq& req)
        {
            const auto plugin = plugins.Get(req.id);

            if (!plugin.has_value())
            {
                throw Error(-1, "Plugin not found");
            }

            const auto metadata_lookup = [&](const char* key) -> std::optional<std::string>
            {
                const auto it = plugin->metadata.find(key);
                if (it == plugin->metadata.end() || !it->second.is_string()) { return std::nullopt; }
                return it->second.get<std::string>();
            };

            const auto owner      = metadata_lookup("owner");
            const auto repository = metadata_lookup("repository");
            const auto version    = metadata_lookup("version");

            if (!metadata_lookup("source") || !owner || !repository || !version)
            {
                throw Error(-2, "Plugin is missing required metadata for upgrading");
            }

            if (*version == req.version)
            {
                throw Error(-3, "Cannot upgrade to same version");
            }

            std::stringstream url;
            url << "https://api.github.com/repos/"
                << owner.value()
                << "/"
                << repository.value()
                << "/releases/tags/" << req.version;

            BOOST_LOG_TRIVIAL(trace) << "Begin upgrading plugin from " << url.str();

            const auto [
                release_status,
                release_body ] = co_await curl.AsyncHttpGet(url.str(), boost::asio::use_awaitable);

            if (release_status != 200)
            {
                throw Error(-4, "Release not found");
            }

            porla::Json::GitHubRelease release;

            try
            {
                release = nlohmann::json::parse(release_body).get<porla::Json::GitHubRelease>();
            }
            catch (const std::exception& e)
            {
                throw Error(-5, "Failed to parse release body as JSON", {{"what", e.what()}});
            }

            if (release.assets.size() == 0)
            {
                throw Error(-6, "Release has no downloadable asset");
            }

            BOOST_LOG_TRIVIAL(info)
                << "Found version "
                << release.tag_name
                << " of plugin - fetching from " << release.assets[0].browser_download_url;

            const auto [
                asset_status,
                asset_body ] = co_await curl.AsyncHttpGet(release.assets[0].browser_download_url, boost::asio::use_awaitable);

            if (asset_status != 200)
            {
                throw Error(-7, "Failed to fetch release asset");
            }

            std::stringstream zip_file_name;
            zip_file_name
                << owner.value()
                << "_"
                << repository.value()
                << "_" << release.tag_name << ".zip";

            const auto plugins_dir = fs::absolute(state_dir / "plugins").lexically_normal();
            const auto plugin_zip  = plugins_dir / zip_file_name.str();

            if (!fs::exists(plugins_dir))
            {
                fs::create_directories(plugins_dir);
            }

            {
                std::ofstream out(plugin_zip, std::ios::binary);

                out.write(
                    asset_body.data(),
                    asset_body.size());

                if (!out)
                {
                    throw Error(-8, "Failed to write plugin archive to " + plugin_zip.string());
                }

                BOOST_LOG_TRIVIAL(debug) << "Wrote plugin to " << plugin_zip;
            }

            const auto old_path = plugin->path;

            auto metadata = plugin->metadata;
            metadata["version"] = release.tag_name;

            plugins.Update(porla::Plugins::Plugin{
                .id       = plugin->id,
                .path     = plugin_zip,
                .config   = plugin->config,
                .metadata = metadata
            });

            std::error_code same_ec;
            if (!fs::equivalent(old_path, plugin_zip, same_ec) && fs::exists(old_path))
            {
                std::error_code ec;

                if (!fs::remove(old_path, ec))
                {
                    BOOST_LOG_TRIVIAL(warning) << "Failed to remove old plugin path " << old_path << ": " << ec;
                }
            }

            co_return;
        }
    }

    void Register(boost::asio::io_context& io, CurlMulti& curl, JsonRpc& jsonrpc, porla::Plugins& plugins, const fs::path& state_dir)
    {
        using Kind = porla::Auth::Context::Kind;

        auto ex = io.get_executor();

        jsonrpc.Register("plugins.add",     { Kind::User },                MakeMethod<AddReq>(ex, std::bind_front(Add, std::ref(plugins))));
        jsonrpc.Register("plugins.get",     { Kind::User, Kind::Machine }, MakeMethod<GetReq>(ex, std::bind_front(Get, std::ref(plugins))));
        jsonrpc.Register("plugins.install", { Kind::User },                MakeMethod<InstallReq>(ex, std::bind_front(Install, std::ref(plugins), std::ref(curl), std::cref(state_dir))));
        jsonrpc.Register("plugins.list",    { Kind::User, Kind::Machine }, MakeMethod<ListReq>(ex, std::bind_front(List, std::ref(plugins))));
        jsonrpc.Register("plugins.reload",  { Kind::User },                MakeMethod<ReloadReq>(ex, std::bind_front(Reload, std::ref(plugins))));
        jsonrpc.Register("plugins.remove",  { Kind::User },                MakeMethod<RemoveReq>(ex, std::bind_front(Remove, std::ref(plugins))));
        jsonrpc.Register("plugins.update",  { Kind::User },                MakeMethod<UpdateReq>(ex, std::bind_front(Update, std::ref(plugins))));
        jsonrpc.Register("plugins.upgrade", { Kind::User },                MakeMethod<UpgradeReq>(ex, std::bind_front(Upgrade, std::ref(plugins), std::ref(curl), std::cref(state_dir))));
    }
}
