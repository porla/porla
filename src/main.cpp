#include <boost/asio.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <cmrc/cmrc.hpp>
#include <curl/curl.h>
#include <sodium.h>

#include "cmdargs.hpp"
#include "config.hpp"
#include "curlmulti.hpp"
#include "events.hpp"
#include "keyvalue.hpp"
#include "logger.hpp"
#include "lua/pluginsource.hpp"
#include "plugins.hpp"
#include "presets.hpp"
#include "sessions/session.hpp"
#include "sessions/sessions.hpp"
#include "sessions/torrent.hpp"
#include "timer.hpp"
#include "torrents.hpp"

#include "auth/authenticator.hpp"

#include "rpc/jsonrpc.hpp"
#include "rpc/methods/auth.hpp"
#include "rpc/methods/fs.hpp"
#include "rpc/methods/kv.hpp"
#include "rpc/methods/mmdb.hpp"
#include "rpc/methods/plugins.hpp"
#include "rpc/methods/presets.hpp"
#include "rpc/methods/sys.hpp"

#include "rpc/methods/sessions/sessionsadd.hpp"
#include "rpc/methods/sessions/sessionsget.hpp"
#include "rpc/methods/sessions/sessionslist.hpp"
#include "rpc/methods/sessions/sessionspause.hpp"
#include "rpc/methods/sessions/sessionsremove.hpp"
#include "rpc/methods/sessions/sessionsresume.hpp"
#include "rpc/methods/sessions/sessionssettingsget.hpp"
#include "rpc/methods/sessions/sessionssettingsset.hpp"
#include "rpc/methods/sessions/sessionsupdate.hpp"
#include "rpc/methods/torrents/torrentaction.hpp"
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
#include "rpc/methods/torrents/torrentspeersadd.hpp"
#include "rpc/methods/torrents/torrentspeerslist.hpp"
#include "rpc/methods/torrents/torrentspiecesget.hpp"
#include "rpc/methods/torrents/torrentspropertiesget.hpp"
#include "rpc/methods/torrents/torrentspropertiesset.hpp"
#include "rpc/methods/torrents/torrentsqueueset.hpp"
#include "rpc/methods/torrents/torrentsremove.hpp"
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

        porla::Events events(io);

        porla::KeyValue kv(porla::KeyValueOptions{
            .db     = cfg->db,
            .events = events
        });

        porla::Presets presets(porla::PresetsOptions{
            .db     = cfg->db,
            .events = events
        });

        auto authenticator       = std::make_shared<porla::Auth::Authenticator>(cfg->db, cfg->secret_key);
        auto curl_multi_instance = porla::CurlMulti::Create(io);
        auto jsonrpc             = porla::Rpc::JsonRpc::Create(authenticator);

        porla::Sessions sessions(porla::SessionsOptions{
            .db     = cfg->db,
            .events = events,
            .io     = io
        });

        porla::Torrents torrents(porla::TorrentsOptions{
            .db       = cfg->db,
            .events   = events,
            .presets  = presets,
            .sessions = sessions
        });

        porla::Plugins plugins(porla::PluginsOptions{
            .cfg         = *cfg,
            .curl_multi  = curl_multi_instance,
            .db          = cfg->db,
            .events      = events,
            .hash_pool   = sodium_hash_pool,
            .http_server = &http_server,
            .jsonrpc     = jsonrpc,
            .io          = io,
            .kv          = kv,
            .presets     = presets,
            .sessions    = sessions,
            .torrents    = torrents
        });

        porla::Rpc::Methods::Auth::Register(io, *jsonrpc, sodium_hash_pool, cfg->db, cfg->secret_key);
        porla::Rpc::Methods::Fs::Register(io, *jsonrpc);
        porla::Rpc::Methods::Kv::Register(io, *jsonrpc, kv);
        porla::Rpc::Methods::Mmdb::Register(io, *jsonrpc, kv, events);
        porla::Rpc::Methods::Plugins::Register(io, *curl_multi_instance, *jsonrpc, plugins, cfg->state_dir);
        porla::Rpc::Methods::Presets::Register(io, *jsonrpc, presets);
        porla::Rpc::Methods::Sys::Register(io, *jsonrpc, cfg->db);

        jsonrpc->Register("sessions.add",              std::make_shared<M::Sessions::SessionsAdd>(sessions));
        jsonrpc->Register("sessions.get",              std::make_shared<M::Sessions::SessionsGet>(sessions));
        jsonrpc->Register("sessions.list",             std::make_shared<M::Sessions::SessionsList>(sessions));
        jsonrpc->Register("sessions.pause",            std::make_shared<M::Sessions::SessionsPause>(sessions));
        jsonrpc->Register("sessions.remove",           std::make_shared<M::Sessions::SessionsRemove>(sessions));
        jsonrpc->Register("sessions.resume",           std::make_shared<M::Sessions::SessionsResume>(sessions));
        jsonrpc->Register("sessions.settings.get",     std::make_shared<M::Sessions::SessionsSettingsGet>(sessions));
        jsonrpc->Register("sessions.settings.set",     std::make_shared<M::Sessions::SessionsSettingsSet>(sessions));
        jsonrpc->Register("sessions.update",           std::make_shared<M::Sessions::SessionsUpdate>(sessions));
        jsonrpc->Register("torrents.add",              std::make_shared<M::Torrents::TorrentsAdd>(torrents));
        jsonrpc->Register("torrents.count",            std::make_shared<M::Torrents::TorrentsCount>(sessions));
        jsonrpc->Register("torrents.files.list",       std::make_shared<M::Torrents::TorrentsFilesList>(sessions));
        jsonrpc->Register("torrents.files.priorities", std::make_shared<M::Torrents::TorrentsFilesPriorities>(sessions));
        jsonrpc->Register("torrents.files.prioritize", std::make_shared<M::Torrents::TorrentsFilesPrioritize>(sessions));
        jsonrpc->Register("torrents.files.progress",   std::make_shared<M::Torrents::TorrentsFilesProgress>(sessions));
        jsonrpc->Register("torrents.files.rename",     std::make_shared<M::Torrents::TorrentsFilesRename>(sessions));
        jsonrpc->Register("torrents.get",              std::make_shared<M::Torrents::TorrentsGet>(sessions));
        jsonrpc->Register("torrents.list",             std::make_shared<M::Torrents::TorrentsList>(sessions));
        jsonrpc->Register("torrents.migrate",          std::make_shared<M::Torrents::TorrentsMigrate>(io, events, sessions));
        jsonrpc->Register("torrents.move",             std::make_shared<M::Torrents::TorrentsMove>(sessions));
        jsonrpc->Register("torrents.pause",            std::make_shared<M::Torrents::TorrentAction>(sessions, [](const auto& r) { r.torrent->status.handle.unset_flags(lt::torrent_flags::auto_managed); r.torrent->status.handle.pause(); }));
        jsonrpc->Register("torrents.peers.add",        std::make_shared<M::Torrents::TorrentsPeersAdd>(sessions));
        jsonrpc->Register("torrents.peers.list",       std::make_shared<M::Torrents::TorrentsPeersList>(sessions));
        jsonrpc->Register("torrents.pieces.get",       std::make_shared<M::Torrents::TorrentsPiecesGet>(sessions));
        jsonrpc->Register("torrents.properties.get",   std::make_shared<M::Torrents::TorrentsPropertiesGet>(sessions));
        jsonrpc->Register("torrents.properties.set",   std::make_shared<M::Torrents::TorrentsPropertiesSet>(sessions, torrents));
        jsonrpc->Register("torrents.queue.bottom",     std::make_shared<M::Torrents::TorrentAction>(sessions, [](const auto& r) { r.torrent->status.handle.queue_position_bottom(); }));
        jsonrpc->Register("torrents.queue.down",       std::make_shared<M::Torrents::TorrentAction>(sessions, [](const auto& r) { r.torrent->status.handle.queue_position_down(); }));
        jsonrpc->Register("torrents.queue.set",        std::make_shared<M::Torrents::TorrentsQueueSet>(sessions));
        jsonrpc->Register("torrents.queue.top",        std::make_shared<M::Torrents::TorrentAction>(sessions, [](const auto& r) { r.torrent->status.handle.queue_position_top(); }));
        jsonrpc->Register("torrents.queue.up",         std::make_shared<M::Torrents::TorrentAction>(sessions, [](const auto& r) { r.torrent->status.handle.queue_position_up(); }));
        jsonrpc->Register("torrents.recheck",          std::make_shared<M::Torrents::TorrentAction>(sessions, [](const auto& r) { r.session->Recheck(r.torrent->status.info_hashes); }));
        jsonrpc->Register("torrents.remove",           std::make_shared<M::Torrents::TorrentsRemove>(sessions));
        jsonrpc->Register("torrents.resume",           std::make_shared<M::Torrents::TorrentAction>(sessions, [](const auto& r) { r.torrent->status.handle.resume(); }));
        jsonrpc->Register("torrents.trackers.add",     std::make_shared<M::Torrents::TorrentsTrackersAdd>(sessions));
        jsonrpc->Register("torrents.trackers.list",    std::make_shared<M::Torrents::TorrentsTrackersList>(sessions));

              auto embedded_core       = porla::Lua::PluginSource{.entrypoint = "plugin.lua", .sources = {}};
        const auto embedded_core_files = cmrc::porla_lua::get_filesystem();

        Traverse(embedded_core_files, "", embedded_core.sources);

        if (embedded_core.sources.find(embedded_core.entrypoint) != embedded_core.sources.end())
        {
            plugins.SetCore(embedded_core);
        }

        sessions.Load(
            [&cfg, &http_server, &jsonrpc, &plugins]()
            {
                try
                {
                    plugins.LoadAll();
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
