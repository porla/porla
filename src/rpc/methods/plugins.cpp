#include "plugins.hpp"

#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>

#include <boost/asio/awaitable.hpp>
#include <boost/log/trivial.hpp>

#include "../functionmethod.hpp"
#include "../jsonrpc.hpp"

#include "../../curlmulti.hpp"
#include "../../json/github.hpp"
#include "../../json/utils.hpp"
#include "../../lua/plugin.hpp"
#include "../../lua/pluginmanifest.hpp"
#include "../../lua/pluginsource.hpp"
#include "../../plugins.hpp"

using boost::asio::awaitable;

namespace fs = std::filesystem;

namespace porla::Rpc::Methods::Plugins
{
    namespace
    {
        struct PluginDto
        {
            int                                   id;
            std::string                           path;
            std::string                           slug;
            std::optional<std::string>            config;
            std::map<std::string, nlohmann::json> metadata;
            std::optional<Lua::PluginManifest>    manifest;
            std::string                           status;
            std::optional<std::string>            error;
        };

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
            PluginDto plugin;
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
            std::vector<PluginDto> plugins;
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
            std::optional<std::string> path;
            Json::Patch<std::string>   config;
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
            PluginDto,
            id,
            path,
            slug,
            config,
            metadata,
            manifest,
            status,
            error)

        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(GetRes, plugin)

        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(ListRes, plugins)

        PluginDto ToDto(const porla::Plugins& plugins, const porla::Plugins::Plugin& plugin, Call& call)
        {
            const auto error   = plugins.Error(plugin.id);
            const auto is_user = call.Caller().kind == Auth::Context::Kind::User;

            return PluginDto{
                .id       = plugin.id,
                .path     = plugin.path,
                .slug     = plugins.Slug(plugin),
                .config   = is_user ? plugin.config : std::nullopt,
                .metadata = plugin.metadata,
                .manifest = plugins.Manifest(plugin.id),
                .status   = plugins.Instance(plugin.id) != nullptr ? "running" : "error",
                .error    = is_user ? error : std::nullopt
            };
        }

        // refuses a plugin whose plugin.json is invalid, or whose slug is invalid or another
        // plugin's, with this method's code
        void CheckPlugin(const porla::Plugins& plugins, const porla::Plugins::Plugin& plugin, int code)
        {
            const auto source = Lua::PluginSource::Load(plugin.path);

            std::optional<Lua::PluginManifest> manifest;

            if (source.has_value())
            {
                try
                {
                    manifest = Lua::PluginManifest::FromSource(*source);
                }
                catch(const std::exception& e)
                {
                    throw Error(code, e.what());
                }
            }

            const auto slug = porla::Plugins::SlugOf(plugin, manifest);

            if (!porla::Plugins::IsValidSlug(slug))
            {
                throw Error(code, "Slug '" + slug + "' is invalid - the name must start with a letter and be 2-32 characters", {{"slug", slug}});
            }

            if (const auto owner = plugins.SlugOwner(slug, plugin.id))
            {
                throw Error(
                    code,
                    "Slug '" + slug + "' is used by " + (*owner == 0 ? "the core plugin" : "another plugin"),
                    {{"slug", slug}, {"id", *owner}});
            }
        }

        // GitHub installs are kept as plugins/github/<owner>_<repo>.zip - no tag, so the file and a
        // slug derived from its name stay the same across versions
        fs::path ReleaseZip(const fs::path& state_dir, const std::string& owner, const std::string& repository)
        {
            return fs::absolute(state_dir / "plugins" / "github" / (owner + "_" + repository + ".zip")).lexically_normal();
        }

        // writes a downloaded release next to where it goes (github/.download/, same file name), so a
        // failed or refused download never touches an installed zip
        fs::path WriteDownload(const fs::path& plugin_zip, std::string_view body, int code)
        {
            const auto download = plugin_zip.parent_path() / ".download" / plugin_zip.filename();

            std::error_code ec;
            fs::create_directories(download.parent_path(), ec);

            std::ofstream out(download, std::ios::binary);
            out.write(body.data(), static_cast<std::streamsize>(body.size()));

            if (!out)
            {
                throw Error(code, "Failed to write plugin archive to " + download.string());
            }

            BOOST_LOG_TRIVIAL(debug) << "Wrote plugin to " << download;

            return download;
        }

        // checks the downloaded release's slug, then moves it in place. the zip it replaces may be
        // the running version's: a loaded plugin holds its sources in memory
        void InstallDownload(const porla::Plugins& plugins, porla::Plugins::Plugin& plugin, const fs::path& plugin_zip, int code)
        {
            std::error_code ec;

            try
            {
                CheckPlugin(plugins, plugin, code);
            }
            catch (const Error&)
            {
                fs::remove(plugin.path, ec);
                throw;
            }

            fs::rename(plugin.path, plugin_zip, ec);

            if (ec)
            {
                fs::remove(plugin.path, ec);
                throw Error(code, "Failed to move plugin archive to " + plugin_zip.string());
            }

            plugin.path = plugin_zip;
        }

        std::optional<porla::Plugins::Plugin> FindByPath(const porla::Plugins& plugins, const fs::path& path, int except_id = -1)
        {
            const auto normalized = [](const fs::path& p)
            {
                std::error_code ec;
                const auto canonical = fs::weakly_canonical(p, ec);
                return ec ? p.lexically_normal() : canonical;
            };

            const auto wanted = normalized(path);

            for (const auto& plugin : plugins.All())
            {
                if (plugin.id != except_id && normalized(plugin.path) == wanted)
                {
                    return plugin;
                }
            }

            return std::nullopt;
        }

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

            if (const auto existing = FindByPath(plugins, plugin_path))
            {
                throw Error(-4, "Plugin path is used by another plugin", {{"id", existing->id}});
            }

            const porla::Plugins::Plugin plugin{
                .id       = -1,
                .path     = plugin_path,
                .config   = req.config,
                .metadata = {}
            };

            CheckPlugin(plugins, plugin, -5);

            const auto plugin_id = plugins.Add(plugin);

            co_return nlohmann::json({
                {"id", plugin_id}
            });
        }

        awaitable<GetRes> Get(porla::Plugins& plugins, const GetReq& req, Call& call)
        {
            const auto plugin = plugins.Get(req.id);

            if (!plugin.has_value())
            {
                throw Error(-1, "Plugin not found");
            }

            co_return GetRes{
                .plugin = ToDto(plugins, *plugin, call)
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
                release = Json::Parse(release_body).get<porla::Json::GitHubRelease>();
            }
            catch (const std::exception& e)
            {
                throw Error(-3, "Failed to parse release body as JSON", {{"what", e.what()}});
            }

            if (release.assets.size() == 0)
            {
                throw Error(-4, "Release has no downloadable asset");
            }

            const auto plugin_zip = ReleaseZip(state_dir, req.owner, req.repository);

            if (const auto existing = FindByPath(plugins, plugin_zip))
            {
                throw Error(-7, "Plugin path is used by another plugin", {{"id", existing->id}});
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

            porla::Plugins::Plugin plugin{
                .id       = -1,
                .path     = WriteDownload(plugin_zip, std::string_view(asset_body.data(), asset_body.size()), -6),
                .config   = req.config,
                .metadata = {
                    {"source", "github"},
                    {"owner", req.owner},
                    {"repository", req.repository},
                    {"version", release.tag_name}
                }
            };

            InstallDownload(plugins, plugin, plugin_zip, -8);

            const auto plugin_id = plugins.Add(plugin);

            co_return nlohmann::json({
                {"id", plugin_id}
            });
        }

        awaitable<ListRes> List(porla::Plugins& plugins, const ListReq& req, Call& call)
        {
            ListRes res = {};

            for (const auto& plugin : plugins.All())
            {
                res.plugins.emplace_back(ToDto(plugins, plugin, call));
            }

            co_return res;
        }

        awaitable<void> Reload(porla::Plugins& plugins, const ReloadReq& req)
        {
            if (!plugins.Get(req.id).has_value())
            {
                throw Error(-1, "Plugin not found");
            }

            plugins.Reload(req.id);

            if (const auto error = plugins.Error(req.id))
            {
                throw Error(-2, "Plugin failed to load: " + *error);
            }

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

            if (req.path.has_value())
            {
                const fs::path plugin_path = req.path.value();

                if (!plugin_path.is_absolute())
                {
                    throw Error(-2, "Plugin path must be absolute");
                }

                if (!fs::exists(plugin_path))
                {
                    throw Error(-3, "Plugin path does not exist");
                }

                if (const auto existing = FindByPath(plugins, plugin_path, plugin->id))
                {
                    throw Error(-4, "Plugin path is used by another plugin", {{"id", existing->id}});
                }

                plugin->path     = plugin_path.lexically_normal();
            }

            if (req.config.present)
            {
                plugin->config = req.config.value;
            }

            CheckPlugin(plugins, *plugin, -5);

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
                release = Json::Parse(release_body).get<porla::Json::GitHubRelease>();
            }
            catch (const std::exception& e)
            {
                throw Error(-5, "Failed to parse release body as JSON", {{"what", e.what()}});
            }

            if (release.assets.size() == 0)
            {
                throw Error(-6, "Release has no downloadable asset");
            }

            const auto plugin_zip = ReleaseZip(state_dir, *owner, *repository);

            if (const auto existing = FindByPath(plugins, plugin_zip, plugin->id))
            {
                throw Error(-9, "Plugin path is used by another plugin", {{"id", existing->id}});
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

            auto metadata = plugin->metadata;
            metadata["version"] = release.tag_name;

            porla::Plugins::Plugin upgraded{
                .id       = plugin->id,
                .path     = WriteDownload(plugin_zip, std::string_view(asset_body.data(), asset_body.size()), -8),
                .config   = plugin->config,
                .metadata = metadata
            };

            // a refused slug leaves the old version installed and loaded
            InstallDownload(plugins, upgraded, plugin_zip, -10);

            plugins.Update(upgraded);

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
