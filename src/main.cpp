#include <boost/asio.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <cmrc/cmrc.hpp>
#include <curl/curl.h>
#include <sodium.h>

#include "cmdargs.hpp"
#include "config.hpp"
#include "curlmulti.hpp"
#include "logger.hpp"
#include "lua/pluginengine.hpp"
#include "lua/pluginsource.hpp"
#include "sessions.hpp"
#include "timer.hpp"

#include "auth/authenticator.hpp"

#include "rpc/jsonrpc.hpp"
#include "rpc/methods/auth/authinit.hpp"
#include "rpc/methods/auth/authkeyscreate.hpp"
#include "rpc/methods/auth/authkeyslist.hpp"
#include "rpc/methods/auth/authkeysremove.hpp"
#include "rpc/methods/auth/authlogin.hpp"
#include "rpc/methods/fs/fsspace.hpp"
#include "rpc/methods/kv/keyvalueget.hpp"
#include "rpc/methods/kv/keyvalueset.hpp"
#include "rpc/methods/mmdb/mmdblookup.hpp"
#include "rpc/methods/plugins/pluginsget.hpp"
#include "rpc/methods/plugins/pluginsadd.hpp"
#include "rpc/methods/plugins/pluginsinstall.hpp"
#include "rpc/methods/plugins/pluginslist.hpp"
#include "rpc/methods/plugins/pluginsreload.hpp"
#include "rpc/methods/plugins/pluginsremove.hpp"
#include "rpc/methods/plugins/pluginsupdate.hpp"
#include "rpc/methods/plugins/pluginsupgrade.hpp"
#include "rpc/methods/presets/presetsget.hpp"
#include "rpc/methods/presets/presetslist.hpp"
#include "rpc/methods/presets/presetsadd.hpp"
#include "rpc/methods/presets/presetsremove.hpp"
#include "rpc/methods/presets/presetsupdate.hpp"
#include "rpc/methods/sessions/sessionsadd.hpp"
#include "rpc/methods/sessions/sessionsget.hpp"
#include "rpc/methods/sessions/sessionslist.hpp"
#include "rpc/methods/sessions/sessionspause.hpp"
#include "rpc/methods/sessions/sessionsremove.hpp"
#include "rpc/methods/sessions/sessionsresume.hpp"
#include "rpc/methods/sessions/sessionssettingsget.hpp"
#include "rpc/methods/sessions/sessionssettingsset.hpp"
#include "rpc/methods/sessions/sessionsupdate.hpp"
#include "rpc/methods/sys/sysstatus.hpp"
#include "rpc/methods/sys/sysversions.hpp"
#include "rpc/methods/torrents/torrentsadd.hpp"
#include "rpc/methods/torrents/torrentscount.hpp"
#include "rpc/methods/torrents/torrentsfileslist.hpp"
#include "rpc/methods/torrents/torrentsfilespriorities.hpp"
#include "rpc/methods/torrents/torrentsfilesprioritize.hpp"
#include "rpc/methods/torrents/torrentsfilesprogress.hpp"
#include "rpc/methods/torrents/torrentsfilesrename.hpp"
#include "rpc/methods/torrents/torrentsget.hpp"
#include "rpc/methods/torrents/torrentslist.hpp"
#include "rpc/methods/torrents/torrentsmigrate.hpp"
#include "rpc/methods/torrents/torrentsmove.hpp"
#include "rpc/methods/torrents/torrentspause.hpp"
#include "rpc/methods/torrents/torrentspeersadd.hpp"
#include "rpc/methods/torrents/torrentspeerslist.hpp"
#include "rpc/methods/torrents/torrentspiecesget.hpp"
#include "rpc/methods/torrents/torrentspropertiesget.hpp"
#include "rpc/methods/torrents/torrentspropertiesset.hpp"
#include "rpc/methods/torrents/torrentsqueueany.hpp"
#include "rpc/methods/torrents/torrentsqueueset.hpp"
#include "rpc/methods/torrents/torrentsrecheck.hpp"
#include "rpc/methods/torrents/torrentsremove.hpp"
#include "rpc/methods/torrents/torrentsresume.hpp"
#include "rpc/methods/torrents/torrentstrackersadd.hpp"
#include "rpc/methods/torrents/torrentstrackerslist.hpp"

CMRC_DECLARE(porla_lua);

extern "C"
{
    // normally called by us_loop_run which we don't use since we run the asio loop ourselves
    void us_internal_free_closed_sockets(struct us_loop_t* loop);
}

static void Traverse(
    const cmrc::embedded_filesystem& fs,
    const std::string& dir,
    std::map<std::string, std::vector<char>>& entries)
{
    for (auto&& entry : fs.iterate_directory(dir))
    {
        const auto path = dir.empty()
            ? entry.filename()
            : dir + "/" + entry.filename();

        if (entry.is_directory())
        {
            Traverse(fs, path, entries);
        }
        else
        {
            const auto file = fs.open(path);
            entries.emplace(path, std::vector<char>(file.begin(), file.end()));
        }
    }
}

namespace M = porla::Rpc::Methods;

int main(int argc, char* argv[])
{
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK)
    {
        std::cerr << "curl_global_init failed";
        return -1;
    }

    if (sodium_init() == -1)
    {
        std::cerr << "sodium_init failed";
        return -1;
    }

    const boost::program_options::variables_map cmd = porla::CmdArgs::Parse(argc, argv);

    if (cmd.count("help"))
    {
        return porla::CmdArgs::Help();
    }

    porla::Logger::Setup(cmd);

    std::unique_ptr<porla::Config> cfg;

    try
    {
        cfg = porla::Config::Load(cmd);
    }
    catch (const std::exception& ex)
    {
        BOOST_LOG_TRIVIAL(fatal) << "Failed to load configuration: " << ex.what();
        return -1;
    }

    boost::asio::io_context  io;
    boost::asio::thread_pool sodium_hash_pool(2);

    {
        boost::asio::signal_set signals(io, SIGINT, SIGTERM);

        signals.async_wait(
            [&io](boost::system::error_code const& ec, int signal)
            {
                BOOST_LOG_TRIVIAL(info) << "Interrupt received (" << signal << ") - stopping...";
                io.stop();
            });

        auto* http_loop = reinterpret_cast<us_loop_t*>(uWS::Loop::get(&io));
        uWS::App http_server;

        boost::signals2::signal<void(const std::unordered_set<std::string>&)> kv_updated_signal;

        auto authenticator       = std::make_shared<porla::Auth::Authenticator>(cfg->db, cfg->secret_key);
        auto curl_multi_instance = porla::CurlMulti::Create(io);
        auto jsonrpc             = porla::Rpc::JsonRpc::Create(authenticator);

        porla::Sessions sessions(porla::SessionsOptions{
            .db = cfg->db,
            .io = io
        });

        porla::Lua::PluginEngine plugin_engine{porla::Lua::PluginEngineOptions{
            .cfg         = *cfg,
            .curl_multi  = curl_multi_instance,
            .db          = cfg->db,
            .hash_pool   = sodium_hash_pool,
            .http_server = &http_server,
            .jsonrpc     = jsonrpc,
            .io          = io,
            .sessions    = sessions
        }};

        jsonrpc->Register("auth.init",                 std::make_shared<M::Auth::AuthInit>(io, sodium_hash_pool, cfg->db));
        jsonrpc->Register("auth.keys.create",          std::make_shared<M::Auth::AuthKeysCreate>(cfg->db));
        jsonrpc->Register("auth.keys.list",            std::make_shared<M::Auth::AuthKeysList>(cfg->db));
        jsonrpc->Register("auth.keys.remove",          std::make_shared<M::Auth::AuthKeysRemove>(cfg->db));
        jsonrpc->Register("auth.login",                std::make_shared<M::Auth::AuthLogin>(io, sodium_hash_pool, cfg->db, cfg->secret_key));
        jsonrpc->Register("fs.space",                  std::make_shared<M::Fs::FsSpace>());
        jsonrpc->Register("kv.get",                    std::make_shared<M::Kv::KeyValueGet>(cfg->db));
        jsonrpc->Register("kv.set",                    std::make_shared<M::Kv::KeyValueSet>(io, cfg->db, kv_updated_signal));
        jsonrpc->Register("mmdb.lookup",               std::make_shared<M::Mmdb::MmdbLookup>(cfg->db, kv_updated_signal));
        jsonrpc->Register("plugins.add",               std::make_shared<M::Plugins::PluginsAdd>(cfg->db, plugin_engine));
        jsonrpc->Register("plugins.get",               std::make_shared<M::Plugins::PluginsGet>(cfg->db, plugin_engine));
        jsonrpc->Register("plugins.install",           std::make_shared<M::Plugins::PluginsInstall>(io, cfg->db, curl_multi_instance, plugin_engine, cfg->state_dir));
        jsonrpc->Register("plugins.list",              std::make_shared<M::Plugins::PluginsList>(cfg->db, plugin_engine));
        jsonrpc->Register("plugins.reload",            std::make_shared<M::Plugins::PluginsReload>(plugin_engine));
        jsonrpc->Register("plugins.remove",            std::make_shared<M::Plugins::PluginsRemove>(cfg->db, plugin_engine));
        jsonrpc->Register("plugins.update",            std::make_shared<M::Plugins::PluginsUpdate>(cfg->db, plugin_engine));
        jsonrpc->Register("plugins.upgrade",           std::make_shared<M::Plugins::PluginsUpgrade>(io, cfg->db, curl_multi_instance, plugin_engine, cfg->state_dir));
        jsonrpc->Register("presets.add",               std::make_shared<M::Presets::PresetsAdd>(cfg->db));
        jsonrpc->Register("presets.get",               std::make_shared<M::Presets::PresetsGet>(cfg->db));
        jsonrpc->Register("presets.list",              std::make_shared<M::Presets::PresetsList>(cfg->db));
        jsonrpc->Register("presets.remove",            std::make_shared<M::Presets::PresetsRemove>(cfg->db));
        jsonrpc->Register("presets.update",            std::make_shared<M::Presets::PresetsUpdate>(cfg->db));
        jsonrpc->Register("sessions.add",              std::make_shared<M::Sessions::SessionsAdd>(cfg->db, sessions));
        jsonrpc->Register("sessions.get",              std::make_shared<M::Sessions::SessionsGet>(cfg->db, sessions));
        jsonrpc->Register("sessions.list",             std::make_shared<M::Sessions::SessionsList>(cfg->db, sessions));
        jsonrpc->Register("sessions.pause",            std::make_shared<M::Sessions::SessionsPause>(cfg->db, sessions));
        jsonrpc->Register("sessions.remove",           std::make_shared<M::Sessions::SessionsRemove>(cfg->db, sessions));
        jsonrpc->Register("sessions.resume",           std::make_shared<M::Sessions::SessionsResume>(cfg->db, sessions));
        jsonrpc->Register("sessions.settings.get",     std::make_shared<M::Sessions::SessionsSettingsGet>(cfg->db, sessions));
        jsonrpc->Register("sessions.settings.set",     std::make_shared<M::Sessions::SessionsSettingsSet>(cfg->db, sessions));
        jsonrpc->Register("sessions.update",           std::make_shared<M::Sessions::SessionsUpdate>(cfg->db, sessions));
        jsonrpc->Register("sys.status",                std::make_shared<M::Sys::SysStatus>(cfg->db));
        jsonrpc->Register("sys.versions",              std::make_shared<M::Sys::SysVersions>());
        jsonrpc->Register("torrents.add",              std::make_shared<M::Torrents::TorrentsAdd>(cfg->db, sessions));
        jsonrpc->Register("torrents.count",            std::make_shared<M::Torrents::TorrentsCount>(sessions));
        jsonrpc->Register("torrents.files.list",       std::make_shared<M::Torrents::TorrentsFilesList>(cfg->db, sessions));
        jsonrpc->Register("torrents.files.priorities", std::make_shared<M::Torrents::TorrentsFilesPriorities>(cfg->db, sessions));
        jsonrpc->Register("torrents.files.prioritize", std::make_shared<M::Torrents::TorrentsFilesPrioritize>(cfg->db, sessions));
        jsonrpc->Register("torrents.files.progress",   std::make_shared<M::Torrents::TorrentsFilesProgress>(cfg->db, sessions));
        jsonrpc->Register("torrents.files.rename",     std::make_shared<M::Torrents::TorrentsFilesRename>(cfg->db, sessions));
        jsonrpc->Register("torrents.get",              std::make_shared<M::Torrents::TorrentsGet>(cfg->db, sessions));
        jsonrpc->Register("torrents.list",             std::make_shared<M::Torrents::TorrentsList>(cfg->db, sessions));
        jsonrpc->Register("torrents.migrate",          std::make_shared<M::Torrents::TorrentsMigrate>(cfg->db, sessions));
        jsonrpc->Register("torrents.move",             std::make_shared<M::Torrents::TorrentsMove>(cfg->db, sessions));
        jsonrpc->Register("torrents.pause",            std::make_shared<M::Torrents::TorrentsPause>(cfg->db, sessions));
        jsonrpc->Register("torrents.peers.add",        std::make_shared<M::Torrents::TorrentsPeersAdd>(cfg->db, sessions));
        jsonrpc->Register("torrents.peers.list",       std::make_shared<M::Torrents::TorrentsPeersList>(cfg->db, sessions));
        jsonrpc->Register("torrents.pieces.get",       std::make_shared<M::Torrents::TorrentsPiecesGet>(cfg->db, sessions));
        jsonrpc->Register("torrents.properties.get",   std::make_shared<M::Torrents::TorrentsPropertiesGet>(cfg->db, sessions));
        jsonrpc->Register("torrents.properties.set",   std::make_shared<M::Torrents::TorrentsPropertiesSet>(cfg->db, sessions));
        jsonrpc->Register("torrents.queue.bottom",     std::make_shared<M::Torrents::TorrentsQueueBottom>(cfg->db, sessions));
        jsonrpc->Register("torrents.queue.down",       std::make_shared<M::Torrents::TorrentsQueueDown>(cfg->db, sessions));
        jsonrpc->Register("torrents.queue.set",        std::make_shared<M::Torrents::TorrentsQueueSet>(cfg->db, sessions));
        jsonrpc->Register("torrents.queue.top",        std::make_shared<M::Torrents::TorrentsQueueTop>(cfg->db, sessions));
        jsonrpc->Register("torrents.queue.up",         std::make_shared<M::Torrents::TorrentsQueueUp>(cfg->db, sessions));
        jsonrpc->Register("torrents.recheck",          std::make_shared<M::Torrents::TorrentsRecheck>(cfg->db, sessions));
        jsonrpc->Register("torrents.remove",           std::make_shared<M::Torrents::TorrentsRemove>(cfg->db, sessions));
        jsonrpc->Register("torrents.resume",           std::make_shared<M::Torrents::TorrentsResume>(cfg->db, sessions));
        jsonrpc->Register("torrents.trackers.add",     std::make_shared<M::Torrents::TorrentsTrackersAdd>(cfg->db, sessions));
        jsonrpc->Register("torrents.trackers.list",    std::make_shared<M::Torrents::TorrentsTrackersList>(cfg->db, sessions));

              auto embedded_core       = porla::Lua::PluginSource{.entrypoint = "plugin.lua", .sources = {}};
        const auto embedded_core_files = cmrc::porla_lua::get_filesystem();

        Traverse(embedded_core_files, "", embedded_core.sources);

        if (embedded_core.sources.find(embedded_core.entrypoint) != embedded_core.sources.end())
        {
            plugin_engine.SetCore(embedded_core);
        }

        sessions.Load(
            [&cfg, &http_server, &jsonrpc, &plugin_engine]()
            {
                try
                {
                    plugin_engine.LoadAll();
                }
                catch (const std::exception& e)
                {
                    BOOST_LOG_TRIVIAL(error) << "Failed to load plugins: " << e.what();
                }

                std::string http_base_path = cfg->http_base_path.value_or("/");
                if (http_base_path.empty())        http_base_path = "/";
                if (http_base_path[0] != '/')      http_base_path = "/" + http_base_path;
                if (http_base_path.ends_with("/")) http_base_path = http_base_path.substr(0, http_base_path.size() - 1);

                http_server.post(http_base_path + "/api/v1/jsonrpc", jsonrpc->HttpHandler());

                http_server.listen(
                    cfg->http_host.value_or("127.0.0.1"),
                    cfg->http_port.value_or(1337),
                    [](const auto* t)
                    {
                        BOOST_LOG_TRIVIAL(info) << "HTTP server listening";
                    });
            });

        const auto sweep_timer = porla::Timer::Create(io, 1000, [http_loop]() { us_internal_free_closed_sockets(http_loop); });

        for (;;)
        {
            try
            {
                io.run();
                break;
            }
            catch(const std::exception& e)
            {
                BOOST_LOG_TRIVIAL(error) << "Unhandled exception in event loop: " << e.what();
            }
        }
    }

    curl_global_cleanup();

    BOOST_LOG_TRIVIAL(info) << "Bye";

    return 0;
}
