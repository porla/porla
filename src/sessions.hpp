#pragma once

#include <filesystem>
#include <map>
#include <memory>
#include <string>

#include <boost/asio.hpp>
#include <boost/signals2.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/settings_pack.hpp>
#include <sqlite3.h>

namespace porla
{
    struct SessionsOptions
    {
        sqlite3*                 db;
        boost::asio::io_context& io;
        int                      timer_dht_stats       = 5000;
        int                      timer_session_stats   = 5000;
        int                      timer_torrent_updates = 1000;
    };

    struct SessionsLoadOptions
    {
        std::string           name;
        std::filesystem::path session_params_file;
        lt::settings_pack     settings;
    };

    class Sessions
    {
    public:
        static bool DisallowedSetting(const std::string& name);

        struct SessionState
        {
            friend class Sessions;

            std::string                                   name;
            std::shared_ptr<lt::session>                  session;
            std::filesystem::path                         session_params_file;
            std::map<lt::info_hash_t, lt::torrent_handle> torrents;

            void Recheck(const lt::info_hash_t& hash);

        private:
            std::map<std::pair<int, libtorrent::info_hash_t>, std::vector<std::function<void()>>> m_oneshot_torrent_callbacks;
        };

        typedef boost::signals2::signal<void(const std::string& session, const libtorrent::info_hash_t&)> InfoHashSignal;
        typedef boost::signals2::signal<void(const std::string& session, const lt::span<const int64_t>&)> SessionStatsSignal;
        typedef boost::signals2::signal<void(const std::string& session, const libtorrent::torrent_handle&)> TorrentHandleSignal;
        typedef boost::signals2::signal<void(const std::string& session, const std::vector<libtorrent::torrent_status>&)> TorrentStatusListSignal;

        explicit Sessions(const SessionsOptions& options);
        ~Sessions();

        std::map<std::string, std::shared_ptr<SessionState>>& All();
        std::shared_ptr<SessionState> Default();
        std::shared_ptr<SessionState> Get(const std::string& name);

        void Load(const SessionsLoadOptions& options);

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
        class Timer;

        void PostDhtStats();
        void PostSessionStats();
        void PostTorrentUpdates();

        void ReadAlerts(const std::shared_ptr<SessionState>& state);

        SessionsOptions m_options;
        std::map<std::string, std::shared_ptr<SessionState>> m_sessions;
        std::vector<Timer> m_timers;

        SessionStatsSignal m_session_stats;
        TorrentStatusListSignal m_state_update;
        TorrentHandleSignal m_storage_moved;
        TorrentHandleSignal m_torrent_added;
        TorrentHandleSignal m_torrent_finished;
        TorrentHandleSignal m_torrent_paused;
        InfoHashSignal m_torrent_removed;
        TorrentHandleSignal m_torrent_resumed;
    };
}
