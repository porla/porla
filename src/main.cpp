#include <boost/asio.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <curl/curl.h>
#include <sodium.h>

#include "cmdargs.hpp"
#include "config.hpp"
#include "curlmulti.hpp"
#include "logger.hpp"
#include "lua/pluginengine.hpp"
#include "sessions.hpp"
#include "webui.hpp"

#include "data/models/keyvaluestore.hpp"

#include "rpc/jsonrpc.hpp"
#include "rpc/methods/auth/authinit.hpp"
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
#include "rpc/methods/torrents/torrentsrecheck.hpp"
#include "rpc/methods/torrents/torrentsremove.hpp"
#include "rpc/methods/torrents/torrentsresume.hpp"
#include "rpc/methods/torrents/torrentstrackerslist.hpp"
#include "rpc/methods/webui/webuiinstall.hpp"

int main(int argc, char* argv[])
{
    curl_global_init(CURL_GLOBAL_DEFAULT);

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

    boost::asio::io_context io;

    {
        uWS::Loop::get(&io);
        uWS::App http_server;

        boost::signals2::signal<void(const std::unordered_set<std::string>&)> kv_updated_signal;

        auto curl_multi_instance = porla::CurlMulti::Create(io);
        auto jsonrpc             = porla::Rpc::JsonRpc::Create(cfg->secret_key);
        auto webui               = porla::WebUI::Create(io, cfg->state_dir, cfg->db, curl_multi_instance);

        porla::Sessions sessions(porla::SessionsOptions{
            .db = cfg->db,
            .io = io
        });

        porla::Lua::PluginEngine plugin_engine{porla::Lua::PluginEngineOptions{
            .curl_multi  = curl_multi_instance,
            .db          = cfg->db,
            .http_server = &http_server,
            .io          = io,
            .sessions    = sessions
        }};

        jsonrpc->Register("auth.init",                 std::make_shared<porla::Rpc::Methods::Auth::AuthInit>(cfg->db));
        jsonrpc->Register("auth.login",                std::make_shared<porla::Rpc::Methods::Auth::AuthLogin>(cfg->db, cfg->secret_key));
        jsonrpc->Register("fs.space",                  std::make_shared<porla::Rpc::Methods::Fs::FsSpace>());
        jsonrpc->Register("kv.get",                    std::make_shared<porla::Rpc::Methods::Kv::KeyValueGet>(cfg->db));
        jsonrpc->Register("kv.set",                    std::make_shared<porla::Rpc::Methods::Kv::KeyValueSet>(io, cfg->db, kv_updated_signal));
        jsonrpc->Register("mmdb.lookup",               std::make_shared<porla::Rpc::Methods::Mmdb::MmdbLookup>(cfg->db, kv_updated_signal));
        jsonrpc->Register("plugins.add",               std::make_shared<porla::Rpc::Methods::Plugins::PluginsAdd>(cfg->db, plugin_engine));
        jsonrpc->Register("plugins.get",               std::make_shared<porla::Rpc::Methods::Plugins::PluginsGet>(cfg->db, plugin_engine));
        jsonrpc->Register("plugins.install",           std::make_shared<porla::Rpc::Methods::Plugins::PluginsInstall>(cfg->db, curl_multi_instance, plugin_engine, cfg->state_dir));
        jsonrpc->Register("plugins.list",              std::make_shared<porla::Rpc::Methods::Plugins::PluginsList>(cfg->db, plugin_engine));
        jsonrpc->Register("plugins.reload",            std::make_shared<porla::Rpc::Methods::Plugins::PluginsReload>(plugin_engine));
        jsonrpc->Register("plugins.remove",            std::make_shared<porla::Rpc::Methods::Plugins::PluginsRemove>(cfg->db, plugin_engine));
        jsonrpc->Register("plugins.update",            std::make_shared<porla::Rpc::Methods::Plugins::PluginsUpdate>(cfg->db, plugin_engine));
        jsonrpc->Register("presets.add",               std::make_shared<porla::Rpc::Methods::Presets::PresetsAdd>(cfg->db));
        jsonrpc->Register("presets.get",               std::make_shared<porla::Rpc::Methods::Presets::PresetsGet>(cfg->db));
        jsonrpc->Register("presets.list",              std::make_shared<porla::Rpc::Methods::Presets::PresetsList>(cfg->db));
        jsonrpc->Register("presets.remove",            std::make_shared<porla::Rpc::Methods::Presets::PresetsRemove>(cfg->db));
        jsonrpc->Register("presets.update",            std::make_shared<porla::Rpc::Methods::Presets::PresetsUpdate>(cfg->db));
        jsonrpc->Register("sessions.add",              std::make_shared<porla::Rpc::Methods::Sessions::SessionsAdd>(cfg->db, sessions));
        jsonrpc->Register("sessions.get",              std::make_shared<porla::Rpc::Methods::Sessions::SessionsGet>(cfg->db, sessions));
        jsonrpc->Register("sessions.list",             std::make_shared<porla::Rpc::Methods::Sessions::SessionsList>(cfg->db, sessions));
        jsonrpc->Register("sessions.pause",            std::make_shared<porla::Rpc::Methods::Sessions::SessionsPause>(cfg->db, sessions));
        jsonrpc->Register("sessions.remove",           std::make_shared<porla::Rpc::Methods::Sessions::SessionsRemove>(cfg->db, sessions));
        jsonrpc->Register("sessions.resume",           std::make_shared<porla::Rpc::Methods::Sessions::SessionsResume>(cfg->db, sessions));
        jsonrpc->Register("sessions.settings.get",     std::make_shared<porla::Rpc::Methods::Sessions::SessionsSettingsGet>(cfg->db, sessions));
        jsonrpc->Register("sessions.settings.set",     std::make_shared<porla::Rpc::Methods::Sessions::SessionsSettingsSet>(cfg->db, sessions));
        jsonrpc->Register("sessions.update",           std::make_shared<porla::Rpc::Methods::Sessions::SessionsUpdate>(cfg->db, sessions));
        jsonrpc->Register("sys.status",                std::make_shared<porla::Rpc::Methods::Sys::SysStatus>(cfg->db));
        jsonrpc->Register("sys.versions",              std::make_shared<porla::Rpc::Methods::Sys::SysVersions>());
        jsonrpc->Register("torrents.add",              std::make_shared<porla::Rpc::Methods::Torrents::TorrentsAdd>(cfg->db, sessions));
        jsonrpc->Register("torrents.count",            std::make_shared<porla::Rpc::Methods::Torrents::TorrentsCount>(sessions));
        jsonrpc->Register("torrents.files.list",       std::make_shared<porla::Rpc::Methods::Torrents::TorrentsFilesList>(cfg->db, sessions));
        jsonrpc->Register("torrents.files.priorities", std::make_shared<porla::Rpc::Methods::Torrents::TorrentsFilesPriorities>(cfg->db, sessions));
        jsonrpc->Register("torrents.files.prioritize", std::make_shared<porla::Rpc::Methods::Torrents::TorrentsFilesPrioritize>(cfg->db, sessions));
        jsonrpc->Register("torrents.files.progress",   std::make_shared<porla::Rpc::Methods::Torrents::TorrentsFilesProgress>(cfg->db, sessions));
        jsonrpc->Register("torrents.files.rename",     std::make_shared<porla::Rpc::Methods::Torrents::TorrentsFilesRename>(cfg->db, sessions));
        jsonrpc->Register("torrents.get",              std::make_shared<porla::Rpc::Methods::Torrents::TorrentsGet>(cfg->db, sessions));
        jsonrpc->Register("torrents.list",             std::make_shared<porla::Rpc::Methods::Torrents::TorrentsList>(cfg->db, sessions));
        jsonrpc->Register("torrents.migrate",          std::make_shared<porla::Rpc::Methods::Torrents::TorrentsMigrate>(cfg->db, sessions));
        jsonrpc->Register("torrents.move",             std::make_shared<porla::Rpc::Methods::Torrents::TorrentsMove>(cfg->db, sessions));
        jsonrpc->Register("torrents.pause",            std::make_shared<porla::Rpc::Methods::Torrents::TorrentsPause>(cfg->db, sessions));
        jsonrpc->Register("torrents.peers.add",        std::make_shared<porla::Rpc::Methods::Torrents::TorrentsPeersAdd>(cfg->db, sessions));
        jsonrpc->Register("torrents.peers.list",       std::make_shared<porla::Rpc::Methods::Torrents::TorrentsPeersList>(cfg->db, sessions));
        jsonrpc->Register("torrents.pieces.get",       std::make_shared<porla::Rpc::Methods::Torrents::TorrentsPiecesGet>(cfg->db, sessions));
        jsonrpc->Register("torrents.properties.get",   std::make_shared<porla::Rpc::Methods::Torrents::TorrentsPropertiesGet>(cfg->db, sessions));
        jsonrpc->Register("torrents.properties.set",   std::make_shared<porla::Rpc::Methods::Torrents::TorrentsPropertiesSet>(cfg->db, sessions));
        jsonrpc->Register("torrents.recheck",          std::make_shared<porla::Rpc::Methods::Torrents::TorrentsRecheck>(cfg->db, sessions));
        jsonrpc->Register("torrents.remove",           std::make_shared<porla::Rpc::Methods::Torrents::TorrentsRemove>(cfg->db, sessions));
        jsonrpc->Register("torrents.resume",           std::make_shared<porla::Rpc::Methods::Torrents::TorrentsResume>(cfg->db, sessions));
        jsonrpc->Register("torrents.trackers.list",    std::make_shared<porla::Rpc::Methods::Torrents::TorrentsTrackersList>(cfg->db, sessions));
        jsonrpc->Register("webui.install",             std::make_shared<porla::Rpc::Methods::WebUI::WebUIInstall>(webui));

        if (!webui->Has())
        {
            webui->Install("latest");
        }

        sessions.LoadAll();

        plugin_engine.LoadAll();

        boost::asio::signal_set signals(io, SIGINT, SIGTERM);

        signals.async_wait(
            [&io, &plugin_engine, &signals](boost::system::error_code const& ec, int signal)
            {
                BOOST_LOG_TRIVIAL(info) << "Interrupt received (" << signal << ") - stopping... Press Ctrl+C again to force";

                plugin_engine.UnloadAll([&io]()
                {
                    io.stop();
                });

                signals.async_wait(
                    [&io](boost::system::error_code const& ec, int signal)
                    {
                        BOOST_LOG_TRIVIAL(warning) << "Second interrupt received (" << signal << ") - forcing shutdown";
                        io.stop();
                    });
            });

        std::string http_base_path = cfg->http_base_path.value_or("/");
        if (http_base_path.empty())        http_base_path = "/";
        if (http_base_path[0] != '/')      http_base_path = "/" + http_base_path;
        if (http_base_path.ends_with("/")) http_base_path = http_base_path.substr(0, http_base_path.size() - 1);

        http_server.post(http_base_path + "/api/v1/jsonrpc", jsonrpc->HttpHandler());
        http_server.get(http_base_path  + "/*",              webui->HttpHandler());

        http_server.listen(
            cfg->http_host.value_or("127.0.0.1"),
            cfg->http_port.value_or(1337),
            [](const auto* t)
            {
                BOOST_LOG_TRIVIAL(info) << "HTTP server listening";
            });

        io.run();
    }

    curl_global_cleanup();

    BOOST_LOG_TRIVIAL(info) << "Bye";

    return 0;
}
