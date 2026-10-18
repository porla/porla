#include "sessions.hpp"

#include <functional>
#include <map>
#include <string>

#include <boost/asio/awaitable.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/settings_pack.hpp>

#include "resolve.hpp"

#include "../functionmethod.hpp"
#include "../jsonrpc.hpp"

#include "../../json/lttypes.hpp"
#include "../../json/utils.hpp"
#include "../../sessions/session.hpp"
#include "../../sessions/sessions.hpp"
#include "../../utils/ltsettings.hpp"

using boost::asio::awaitable;

namespace porla::Rpc::Methods::Sessions
{
    namespace
    {
        static const std::map<std::string, std::function<lt::settings_pack()>> packs
        {
            { "default", &lt::default_settings },
            { "min_memory_usage", &lt::min_memory_usage },
            { "high_performance_seed", &lt::high_performance_seed }
        };

        struct SessionRef
        {
            int id;
        };

        struct AddReq
        {
            std::string                                          name;
            std::optional<std::map<std::string, nlohmann::json>> metadata;
            std::optional<std::string>                           settings_base;
            std::optional<std::map<std::string, nlohmann::json>> settings;
            std::optional<int>                                   timer_dht_stats;
            std::optional<int>                                   timer_save_state;
            std::optional<int>                                   timer_session_stats;
            std::optional<int>                                   timer_torrent_updates;
        };

        struct GetRes
        {
            struct SessionState
            {
                bool  is_listening;
                bool  is_paused;
                int   torrents_total;
            };

            struct Session
            {
                int                                   id;
                std::string                           name;
                bool                                  is_default;
                std::map<std::string, nlohmann::json> metadata;
                std::optional<SessionState>           state;
            };

            Session session;
        };

        struct ListReq
        {
        };

        struct ListRes
        {
            struct SessionState
            {
                bool  is_listening;
                bool  is_paused;
                int   torrents_total;
            };

            struct Session
            {
                int                                   id;
                std::string                           name;
                bool                                  is_default;
                std::map<std::string, nlohmann::json> metadata;
                std::optional<SessionState>           state;
            };

            std::vector<Session> sessions;
        };

        struct SettingsSetReq
        {
            int                       id;
            libtorrent::settings_pack settings;
        };

        struct UpdateReq
        {
            int                                   id;
            std::string                           name;
            bool                                  is_default;
            std::map<std::string, nlohmann::json> metadata;
            std::optional<int>                    timer_dht_stats;
            std::optional<int>                    timer_save_state;
            std::optional<int>                    timer_session_stats;
            std::optional<int>                    timer_torrent_updates;
        };

        PORLA_JSON_REQUEST(SessionRef, id)

        PORLA_JSON_REQUEST(
            AddReq,
            name,
            metadata,
            settings_base,
            settings,
            timer_dht_stats,
            timer_save_state,
            timer_session_stats,
            timer_torrent_updates)

        PORLA_JSON_NO_PARAMS(ListReq)
        PORLA_JSON_REQUEST(SettingsSetReq, id, settings)

        PORLA_JSON_REQUEST(
            UpdateReq,
            id,
            name,
            is_default,
            metadata,
            timer_dht_stats,
            timer_save_state,
            timer_session_stats,
            timer_torrent_updates)

        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
            GetRes::SessionState,
            is_listening,
            is_paused,
            torrents_total)

        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
            GetRes::Session,
            id,
            name,
            is_default,
            metadata,
            state)

        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
            GetRes,
            session)

        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
            ListRes,
            sessions)

        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
            ListRes::SessionState,
            is_listening,
            is_paused,
            torrents_total)

        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
            ListRes::Session,
            id,
            name,
            is_default,
            metadata,
            state)

        awaitable<nlohmann::json> Add(porla::Sessions& sessions, const AddReq& req)
        {
            std::string settings_base = req.settings_base.value_or("default");

            const auto found_pack = packs.find(settings_base);

            if (found_pack == packs.end())
            {
                throw Error(-32602, "Invalid session settings base", {{"field","settings_base"}});
            }

            if (req.timer_dht_stats && *req.timer_dht_stats < 10)
            {
                throw Error(-32602, "Too narrow timer interval", {{"field","timer_dht_stats"}});
            }

            if (req.timer_save_state && *req.timer_save_state < 10)
            {
                throw Error(-32602, "Too narrow timer interval", {{"field","timer_save_state"}});
            }

            if (req.timer_session_stats && *req.timer_session_stats < 10)
            {
                throw Error(-32602, "Too narrow timer interval", {{"field","timer_session_stats"}});
            }

            if (req.timer_torrent_updates && *req.timer_torrent_updates < 10)
            {
                throw Error(-32602, "Too narrow timer interval", {{"field","timer_torrent_updates"}});
            }

            lt::settings_pack settings = found_pack->second();

            Utils::LibtorrentSettingsPack::Update(
                settings,
                req.settings.value_or(std::map<std::string, nlohmann::json>()));

            const auto session = porla::Sessions::Record{
                .id                    = -1,
                .name                  = req.name,
                .is_default            = false,
                .metadata              = req.metadata.value_or(std::map<std::string, nlohmann::json>()),
                .params                = lt::session_params(settings),
                .timer_dht_stats       = req.timer_dht_stats.value_or(5000),
                .timer_save_state      = req.timer_save_state.value_or(300000),
                .timer_session_stats   = req.timer_session_stats.value_or(5000),
                .timer_torrent_updates = req.timer_torrent_updates.value_or(1000)
            };

            co_return nlohmann::json({
                {"id", sessions.Add(session)}
            });
        }

        awaitable<GetRes> Get(porla::Sessions& sessions, const SessionRef& req)
        {
            const auto session = FindSession(sessions, req.id);
            const auto& state  = sessions.Get(req.id);

            co_return GetRes{
                .session = GetRes::Session{
                    .id         = session.id,
                    .name       = session.name,
                    .is_default = session.is_default,
                    .metadata   = session.metadata,
                    .state      = state == nullptr
                        ? std::optional<GetRes::SessionState>()
                        : GetRes::SessionState{
                            .is_listening   = state->Libtorrent().is_listening(),
                            .is_paused      = state->Libtorrent().is_paused(),
                            .torrents_total = static_cast<int>(state->Count())
                        }
                }
            };
        }

        awaitable<ListRes> List(porla::Sessions& sessions, const ListReq& req)
        {
            const auto& avail = sessions.List();

            std::vector<ListRes::Session> session_items;

            for (const auto& session : avail)
            {
                const auto state = sessions.Get(session.id);

                session_items.push_back(ListRes::Session{
                    .id         = session.id,
                    .name       = session.name,
                    .is_default = session.is_default,
                    .metadata   = session.metadata,
                    .state      = state == nullptr
                        ? std::optional<ListRes::SessionState>()
                        : ListRes::SessionState{
                            .is_listening   = state->Libtorrent().is_listening(),
                            .is_paused      = state->Libtorrent().is_paused(),
                            .torrents_total = static_cast<int>(state->Count())
                        }
                });
            }

            co_return ListRes{
                .sessions = session_items
            };
        }

        awaitable<void> Pause(porla::Sessions& sessions, const SessionRef& req)
        {
            const auto session = ResolveSession(sessions, req.id);
            session.session->Libtorrent().pause();
            co_return;
        }

        awaitable<void> Remove(porla::Sessions& sessions, const SessionRef& req)
        {
            const auto session = FindSession(sessions, req.id);

            if (session.is_default)
            {
                throw Error(-2, "Cannot remove default session");
            }

            const auto session_state = sessions.Get(session.id);

            if (sessions.CountParams(session.id) > 0 || (session_state != nullptr && !session_state->Empty()))
            {
                throw Error(-3, "Cannot remove session with torrents");
            }

            sessions.Remove(session.id);

            co_return;
        }

        awaitable<void> Resume(porla::Sessions& sessions, const SessionRef& req)
        {
            const auto session = ResolveSession(sessions, req.id);
            session.session->Libtorrent().resume();
            co_return;
        }

        awaitable<nlohmann::json> SettingsGet(porla::Sessions& sessions, const SessionRef &req, Call& call)
        {
            const auto  session = FindSession(sessions, req.id);
            const auto& state   = sessions.Get(session.id);

            nlohmann::json settings = state == nullptr
                ? session.params.settings
                : state->Libtorrent().get_settings();

            if (call.Caller().kind != Auth::Context::Kind::User)
            {
                settings["proxy_password"] = nullptr;
            }

            co_return nlohmann::json({
                {"settings", settings}
            });
        }

        awaitable<void> SettingsSet(porla::Sessions& sessions, const SettingsSetReq& req)
        {
            const auto resolved = ResolveSession(sessions, req.id);

            lt::settings_pack settings = req.settings;

            Utils::LibtorrentSettingsPack::UpdateStatic(settings);

            resolved.session->Libtorrent().apply_settings(settings);
            resolved.session->Persist();

            BOOST_LOG_TRIVIAL(info) << "Session settings for " << resolved.session->Name() << " updated";

            co_return;
        }

        awaitable<void> Update(porla::Sessions& sessions, const UpdateReq& req)
        {
            auto session = FindSession(sessions, req.id);

            if (req.timer_dht_stats && *req.timer_dht_stats < 10)
            {
                throw Error(-32602, "Too narrow timer interval", {{"field","timer_dht_stats"}});
            }

            if (req.timer_save_state && *req.timer_save_state < 10)
            {
                throw Error(-32602, "Too narrow timer interval", {{"field","timer_save_state"}});
            }

            if (req.timer_session_stats && *req.timer_session_stats < 10)
            {
                throw Error(-32602, "Too narrow timer interval", {{"field","timer_session_stats"}});
            }

            if (req.timer_torrent_updates && *req.timer_torrent_updates < 10)
            {
                throw Error(-32602, "Too narrow timer interval", {{"field","timer_torrent_updates"}});
            }

            session.name                  = req.name;
            session.is_default            = req.is_default;
            session.metadata              = req.metadata;
            session.timer_dht_stats       = req.timer_dht_stats.value_or(session.timer_dht_stats);
            session.timer_save_state      = req.timer_save_state.value_or(session.timer_save_state);
            session.timer_session_stats   = req.timer_session_stats.value_or(session.timer_session_stats);
            session.timer_torrent_updates = req.timer_torrent_updates.value_or(session.timer_torrent_updates);

            sessions.Update(session);

            co_return;
        }
    }

    void Register(boost::asio::io_context& io, JsonRpc& jsonrpc, porla::Sessions& sessions)
    {
        using Kind = porla::Auth::Context::Kind;

        auto ex = io.get_executor();

        jsonrpc.Register("sessions.add",          { Kind::User },                MakeMethod<AddReq>(ex, std::bind_front(Add, std::ref(sessions))));
        jsonrpc.Register("sessions.get",          { Kind::User, Kind::Machine }, MakeMethod<SessionRef>(ex, std::bind_front(Get, std::ref(sessions))));
        jsonrpc.Register("sessions.list",         { Kind::User, Kind::Machine }, MakeMethod<ListReq>(ex, std::bind_front(List, std::ref(sessions))));
        jsonrpc.Register("sessions.pause",        { Kind::User },                MakeMethod<SessionRef>(ex, std::bind_front(Pause, std::ref(sessions))));
        jsonrpc.Register("sessions.remove",       { Kind::User },                MakeMethod<SessionRef>(ex, std::bind_front(Remove, std::ref(sessions))));
        jsonrpc.Register("sessions.resume",       { Kind::User },                MakeMethod<SessionRef>(ex, std::bind_front(Resume, std::ref(sessions))));
        jsonrpc.Register("sessions.settings.get", { Kind::User, Kind::Machine }, MakeMethod<SessionRef>(ex, std::bind_front(SettingsGet, std::ref(sessions))));
        jsonrpc.Register("sessions.settings.set", { Kind::User },                MakeMethod<SettingsSetReq>(ex, std::bind_front(SettingsSet, std::ref(sessions))));
        jsonrpc.Register("sessions.update",       { Kind::User },                MakeMethod<UpdateReq>(ex, std::bind_front(Update, std::ref(sessions))));
    }
}
