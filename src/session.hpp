#pragma once

#include <map>
#include <memory>
#include <vector>

#include <boost/asio.hpp>
#include <boost/signals2.hpp>
#include <libtorrent/session.hpp>
#include <sqlite3.h>

namespace porla
{
    struct SessionOptions
    {
        sqlite3* db = nullptr;
        lt::settings_pack settings = lt::default_settings();
        int timer_dht_stats = 5000;
        int timer_session_stats = 5000;
        int timer_torrent_updates = 1000;
    };

    class ISession
    {
    public:
        typedef boost::signals2::signal<void(const libtorrent::info_hash_t&)> InfoHashSignal;
        typedef boost::signals2::signal<void(const std::map<std::string, int64_t>&)> SessionStatsSignal;
        typedef boost::signals2::signal<void(const libtorrent::torrent_status&)> TorrentStatusSignal;
        typedef boost::signals2::signal<void(const std::vector<libtorrent::torrent_status>&)> TorrentStatusListSignal;

        virtual boost::signals2::connection OnSessionStats(const SessionStatsSignal::slot_type& subscriber) = 0;
        virtual boost::signals2::connection OnStateUpdate(const TorrentStatusListSignal::slot_type& subscriber) = 0;
        virtual boost::signals2::connection OnTorrentAdded(const TorrentStatusSignal::slot_type& subscriber) = 0;
        virtual boost::signals2::connection OnTorrentFinished(const TorrentStatusSignal::slot_type& subscriber) = 0;
        virtual boost::signals2::connection OnTorrentPaused(const TorrentStatusSignal::slot_type& subscriber) = 0;
        virtual boost::signals2::connection OnTorrentRemoved(const InfoHashSignal::slot_type& subscriber) = 0;
        virtual boost::signals2::connection OnTorrentResumed(const TorrentStatusSignal::slot_type& subscriber) = 0;

        virtual void AddTorrent(libtorrent::add_torrent_params const& p) = 0;
        virtual void ForEach(const std::function<void(const libtorrent::torrent_status&)>& cb) = 0;
        virtual void Query(const std::string_view& query, const std::function<int(sqlite3_stmt*)>& cb) = 0;
        virtual void Remove(const lt::info_hash_t& hash) = 0;
        virtual const std::map<lt::info_hash_t, lt::torrent_status>& Torrents() = 0;
    };

    class Session : public ISession
    {
    public:
        explicit Session(boost::asio::io_context& io, SessionOptions const& options);

        Session(const Session&) = delete;
        Session(Session&&) = delete;
        Session& operator=(const Session&) = delete;
        Session& operator=(Session&&) = delete;

        ~Session();

        boost::signals2::connection OnSessionStats(const SessionStatsSignal::slot_type& subscriber) override
        {
            return m_sessionStats.connect(subscriber);
        }

        boost::signals2::connection OnStateUpdate(const TorrentStatusListSignal::slot_type& subscriber) override
        {
            return m_stateUpdate.connect(subscriber);
        }

        boost::signals2::connection OnTorrentAdded(const TorrentStatusSignal::slot_type& subscriber) override
        {
            return m_torrentAdded.connect(subscriber);
        }

        boost::signals2::connection OnTorrentFinished(const TorrentStatusSignal::slot_type& subscriber) override
        {
            return m_torrentFinished.connect(subscriber);
        }

        boost::signals2::connection OnTorrentPaused(const TorrentStatusSignal::slot_type& subscriber) override
        {
            return m_torrentPaused.connect(subscriber);
        }

        boost::signals2::connection OnTorrentRemoved(const InfoHashSignal::slot_type& subscriber) override
        {
            return m_torrentRemoved.connect(subscriber);
        }

        boost::signals2::connection OnTorrentResumed(const TorrentStatusSignal::slot_type& subscriber) override
        {
            return m_torrentResumed.connect(subscriber);
        }

        void Load();

        void AddTorrent(libtorrent::add_torrent_params const& p) override;
        void ForEach(const std::function<void(const libtorrent::torrent_status&)>& cb) override;
        void Query(const std::string_view& query, const std::function<int(sqlite3_stmt*)>& cb) override;
        void Remove(const lt::info_hash_t& hash) override;
        const std::map<lt::info_hash_t, lt::torrent_status>& Torrents() override;

    private:
        class Timer;

        void ReadAlerts();

        boost::asio::io_context& m_io;
        std::vector<Timer> m_timers;
        std::vector<lt::stats_metric> m_stats;

        SessionStatsSignal m_sessionStats;
        TorrentStatusListSignal m_stateUpdate;
        TorrentStatusSignal m_torrentAdded;
        TorrentStatusSignal m_torrentFinished;
        TorrentStatusSignal m_torrentPaused;
        InfoHashSignal m_torrentRemoved;
        TorrentStatusSignal m_torrentResumed;

        sqlite3* m_db;
        sqlite3* m_tdb;

        std::unique_ptr<libtorrent::session> m_session;
        std::map<libtorrent::info_hash_t, libtorrent::torrent_status> m_torrents;
    };
}
