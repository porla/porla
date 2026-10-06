#pragma once

#include <filesystem>
#include <map>
#include <memory>
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
        struct TorrentFileErrorEvent
        {
            std::string        file;
            lt::torrent_handle torrent;
        };

        struct Event
        {
            std::string                           name;
            std::map<std::string, nlohmann::json> data              = std::map<std::string, nlohmann::json>();
            int                                   session_id        = -1;
            lt::torrent_handle                    torrent           = {};
            lt::info_hash_t                       torrent_info_hash;
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
        typedef boost::signals2::signal<void(SessionStatePtr, const lt::span<const int64_t>&)> SessionStatsSignal;
        typedef boost::signals2::signal<void(SessionStatePtr, const TorrentFileErrorEvent&)> TorrentFileErrorSignal;
        typedef boost::signals2::signal<void(SessionStatePtr, const lt::torrent_handle&)> TorrentHandleSignal;
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

        boost::signals2::connection OnEvent(const EventSignal::slot_type& subscriber)
        {
            return m_event.connect(subscriber);
        }

        boost::signals2::connection OnSessionStats(const SessionStatsSignal::slot_type& subscriber)
        {
            return m_session_stats.connect(subscriber);
        }

        boost::signals2::connection OnStateUpdate(const TorrentStatusListSignal::slot_type& subscriber)
        {
            return m_state_update.connect(subscriber);
        }

        boost::signals2::connection OnStorageMoved(const TorrentHandleSignal::slot_type& subscriber)
        {
            return m_storage_moved.connect(subscriber);
        }

        boost::signals2::connection OnTorrentAdded(const TorrentHandleSignal::slot_type& subscriber)
        {
            return m_torrent_added.connect(subscriber);
        }

        boost::signals2::connection OnTorrentFileError(const TorrentFileErrorSignal::slot_type& subscriber)
        {
            return m_torrent_file_error.connect(subscriber);
        }

        boost::signals2::connection OnTorrentFinished(const TorrentHandleSignal::slot_type& subscriber)
        {
            return m_torrent_finished.connect(subscriber);
        }

        boost::signals2::connection OnTorrentPaused(const TorrentHandleSignal::slot_type& subscriber)
        {
            return m_torrent_paused.connect(subscriber);
        }

        boost::signals2::connection OnTorrentRemoved(const InfoHashSignal::slot_type& subscriber)
        {
            return m_torrent_removed.connect(subscriber);
        }

        boost::signals2::connection OnTorrentResumed(const TorrentHandleSignal::slot_type& subscriber)
        {
            return m_torrent_resumed.connect(subscriber);
        }

    private:
        void Publish(const SessionStatePtr& state, Event event)
        {
            if (m_event.empty()) { return; }

            event.session_id = state->id;

            boost::asio::post(
                m_options.io,
                [this, weak = std::weak_ptr(state), event = std::move(event)]()
                {
                    if (auto state = weak.lock()) { m_event(state, event); }
                });
        }

        void PublishDetached(int session_id, Event event)
        {
            if (m_event.empty()) { return; }

            event.session_id = session_id;

            boost::asio::post(
                m_options.io,
                [this, event = std::move(event)]()
                {
                    m_event(nullptr, event);
                });
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

        template <typename Signal, typename... Args>
        void Emit(Signal& signal, std::shared_ptr<SessionState> state, Args... args)
        {
            boost::asio::post(
                m_options.io,
                [&signal, weak = std::weak_ptr(state), args...]()
                {
                    if (auto state = weak.lock()) { signal(state, args...); }
                });
        }

        SessionsOptions m_options;
        std::map<int, SessionStatePtr> m_sessions;

        EventSignal m_event;
        SessionStatsSignal m_session_stats;
        TorrentStatusListSignal m_state_update;
        TorrentHandleSignal m_storage_moved;
        TorrentHandleSignal m_torrent_added;
        TorrentFileErrorSignal m_torrent_file_error;
        TorrentHandleSignal m_torrent_finished;
        TorrentHandleSignal m_torrent_paused;
        InfoHashSignal m_torrent_removed;
        TorrentHandleSignal m_torrent_resumed;
    };
}
