#pragma once

#include <filesystem>
#include <map>
#include <memory>
#include <string_view>
#include <string>

#include <boost/asio.hpp>
#include <boost/signals2.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/settings_pack.hpp>
#include <nlohmann/json.hpp>
#include <sqlite3.h>

#include "data/models/sessions.hpp"

namespace porla
{
    class Timer;

    struct SessionsOptions
    {
        sqlite3*                 db;
        boost::asio::io_context& io;
    };

    class Sessions
    {
    public:
        struct Event
        {
            std::string        name;
            nlohmann::json     data              = nlohmann::json::object();
            int                session_id        = -1;
            lt::torrent_handle torrent           = {};
            lt::info_hash_t    torrent_info_hash;
            std::uint64_t      id                = 0;
        };

        struct SessionState
        {
            friend class Sessions;

            int                                           id;
            std::string                                   name;
            std::unique_ptr<lt::session>                  session;
            std::map<lt::info_hash_t, lt::torrent_status> torrents;

            void Recheck(const lt::info_hash_t& hash);

        private:
            struct LoadState;

            std::unique_ptr<LoadState>          m_load_state;
            std::vector<std::shared_ptr<Timer>> m_timers;
            std::unordered_set<lt::info_hash_t> m_adding;
            std::map<std::pair<int, lt::info_hash_t>, std::vector<std::function<void(const std::shared_ptr<SessionState>&)>>> m_oneshot_torrent_callbacks;
        };

        using SessionStatePtr = std::shared_ptr<SessionState>;

        // SessionStatePtr will be null on session.removed and session.unloaded - use session_id on those.
        typedef boost::signals2::signal<void(SessionStatePtr, const Event&)> EventSignal;

        typedef boost::signals2::signal<void(SessionStatePtr, const lt::info_hash_t&)> InfoHashSignal;
        typedef boost::signals2::signal<void(SessionStatePtr, const std::vector<lt::torrent_status>&)> TorrentStatusListSignal;

        explicit Sessions(const SessionsOptions& options);
        ~Sessions();

        std::map<int, SessionStatePtr> All() { return m_sessions; }
        SessionStatePtr Get(const int id);

        int  Add(const Data::Models::Sessions::Session& session);
        void Remove(int id);
        void Update(const Data::Models::Sessions::Session& session);

        void Load(const std::function<void()>& callback = {});
        void LoadById(int id, const std::function<void()>& callback = {});
        void SaveSessionParams(const SessionStatePtr& state);
        void UnloadById(int id);

        boost::signals2::connection OnEvent(std::string_view name, const EventSignal::slot_type& subscriber)
        {
            auto it = m_events.find(name);

            if (it == m_events.end())
            {
                it = m_events.emplace(std::string(name), std::make_unique<EventSignal>()).first;
            }

            return it->second->connect(subscriber);
        }

        boost::signals2::connection OnStateUpdate(const TorrentStatusListSignal::slot_type& subscriber)
        {
            return m_state_update.connect(subscriber);
        }

        boost::signals2::connection OnTorrentRemoved(const InfoHashSignal::slot_type& subscriber)
        {
            return m_torrent_removed.connect(subscriber);
        }

    private:
        bool HasSubscribers(std::string_view name) const
        {
            const auto it = m_events.find(name);
            return it != m_events.end() && !it->second->empty();
        }

        void Publish(const SessionStatePtr& state, Event event)
        {
            if (!HasSubscribers(event.name)) { return; }

            event.session_id = state->id;
            event.id         = ++m_last_event_id;

            boost::asio::post(
                m_options.io,
                [this, weak = std::weak_ptr(state), event = std::move(event)]()
                {
                    if (auto state = weak.lock()) { Deliver(state, event); }
                });
        }

        void PublishDetached(int session_id, Event event)
        {
            if (!HasSubscribers(event.name)) { return; }

            event.session_id = session_id;
            event.id         = ++m_last_event_id;

            boost::asio::post(
                m_options.io,
                [this, event = std::move(event)]()
                {
                    Deliver(nullptr, event);
                });
        }

        void Deliver(const SessionStatePtr& state, const Event& event)
        {
            // looked up again - subscribers may have come or gone since the event was queued
            if (const auto it = m_events.find(event.name); it != m_events.end())
            {
                (*it->second)(state, event);
            }
        }

        void LoadTorrentsChunk(const SessionStatePtr& state);
        void FinishLoad(const SessionStatePtr& state);

        void PostDhtStats(const SessionStatePtr& state);
        void PostSessionStats(const SessionStatePtr& state);
        void PostTorrentUpdates(const SessionStatePtr& state);

        void ReadAlerts(const SessionStatePtr& state);
        void ProcessAlert(const SessionStatePtr& state, const lt::alert* alert);

        void OnAddTorrentAlert(const SessionStatePtr& state, const lt::add_torrent_alert* alert);
        void OnFileErrorAlert(const SessionStatePtr& state, const lt::file_error_alert* alert);
        void OnSaveResumeDataAlert(const SessionStatePtr& state, const lt::save_resume_data_alert* alert);
        void OnTorrentRemovedAlert(const SessionStatePtr& state, const lt::torrent_removed_alert* alert);
        void OnTorrentResumedAlert(const SessionStatePtr& state, const lt::torrent_resumed_alert* alert);

        void SaveState(const SessionStatePtr& state);
        void UnloadSession(const SessionStatePtr& state);

        template<typename Alert>
        void EmitTorrentEvent(const SessionStatePtr& state, std::string name, const Alert& alert, nlohmann::json extra = {});

        SessionsOptions m_options;
        std::map<int, SessionStatePtr> m_sessions;

        std::map<std::string, std::unique_ptr<EventSignal>, std::less<>> m_events;
        std::uint64_t m_last_event_id = 0;

        TorrentStatusListSignal m_state_update;
        InfoHashSignal m_torrent_removed;
    };
}
