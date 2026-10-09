#pragma once

#include <map>
#include <memory>
#include <string>

#include <boost/asio/io_context.hpp>
#include <libtorrent/session.hpp>
#include <nlohmann/json.hpp>
#include <sqlite3.h>

#include "../data/models/sessions.hpp"

namespace porla
{
    class Events;
    class Timer;

    struct SessionOptions
    {
        Data::Models::Sessions::Session record;
        sqlite3*                        db;
        Events&                         events;
        boost::asio::io_context&        io;
    };

    // maps 1-1 with a libtorrent session
    class Session : public std::enable_shared_from_this<Session>
    {
    public:
        explicit Session(const SessionOptions& options);
        ~Session();

        Session(const Session&)            = delete;
        Session& operator=(const Session&) = delete;

        int                Id()   const             { return m_id; }
        const std::string& Name() const             { return m_name; }
        void               Rename(std::string name) { m_name = std::move(name); }

        lt::session& Libtorrent() { return *m_session; }

        const std::map<lt::info_hash_t, lt::torrent_status>& Torrents() const { return m_torrents; }

        // starts/loads the session. async, load_callback will be called
        // when session is fully loaded.
        void Start(std::function<void()> load_callback = {});

        // stops/unloads the session
        void Stop();

         // stores session params into the sessions table
        void Persist();

        // rechecks the torrent with the given hash. it adds some lipstick
        // to libtorrent checking by actually resuming, etc
        void Recheck(const lt::info_hash_t& hash);

    private:
        struct LoadState;

        // loads the next chunk of torrents
        void LoadNextChunk();

        // completes the loading. called once all chunks
        // have loaded
        void LoadComplete();

        void PostDhtStats();
        void PostSessionStats();
        void PostTorrentUpdates();
        void SaveState();

        void ReadAlerts();
        void ProcessAlert(const lt::alert* alert);

        void OnAddTorrentAlert(const lt::add_torrent_alert* alert);
        void OnFileErrorAlert(const lt::file_error_alert* alert);
        void OnSaveResumeDataAlert(const lt::save_resume_data_alert* alert);
        void OnTorrentRemovedAlert(const lt::torrent_removed_alert* alert);
        void OnTorrentResumedAlert(const lt::torrent_resumed_alert* alert);

        // called whenever the torrents needs to be reconciled (i.e needs sync
        // with the actual lt session). can be called multiple times
        void ScheduleReconcileTorrents();

        // reconciles the torrents
        void ReconcileTorrents();

        // removes any torrent handles that are invalid (is_valid()=false)
        void UntrackInvalidTorrents();

        // untracks a torrent from our end
        void UntrackTorrent(const lt::info_hash_t& hash);

        // whenever a torrent info hash is updated, for example due to metadata
        // turning it from a v1 into a hybrid
        void UpdateInfoHashes(const lt::info_hash_t& prev, const lt::info_hash_t& curr);

        template<typename T>
        void Publish(T event);

        template<typename Alert>
        void EmitTorrentEvent(std::string name, const Alert& alert, nlohmann::json extra = {});

        std::string Log() const;
        std::string Log(const lt::info_hash_t& hash) const;

        int            m_id;
        std::string    m_name;
        SessionOptions m_options;

        std::unique_ptr<lt::session>                  m_session;
        std::map<lt::info_hash_t, lt::torrent_status> m_torrents;

        std::unique_ptr<LoadState>          m_load_state;
        std::vector<std::shared_ptr<Timer>> m_timers;
        std::unordered_set<lt::info_hash_t> m_adding;
        bool                                m_reconcile_pending = false;

        std::map<std::pair<int, lt::info_hash_t>, std::vector<std::function<void(Session&)>>> m_oneshot_torrent_callbacks;
    };
}
