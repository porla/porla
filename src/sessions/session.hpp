#pragma once

#include <map>
#include <memory>
#include <optional>
#include <ranges>
#include <string>

#include <boost/asio/io_context.hpp>
#include <libtorrent/session.hpp>
#include <nlohmann/json.hpp>
#include <sqlite3.h>

#include "../data/models/addtorrentparams.hpp"
#include "../data/models/sessions.hpp"
#include "torrent.hpp"

namespace porla
{
    class Events;
    class Scheduler;
    struct Torrent;

    struct SessionOptions
    {
        Data::Models::Sessions::Session record;
        sqlite3*                        db;
        Events&                         events;
        boost::asio::io_context&        io;
    };

    inline constexpr auto IsCurrent = [](const auto& entry)
    {
        return entry.second.state == Torrent::State::Current;
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

        // Torrent lookup functions since we only want to return current torrents
        const Torrent* Find(const lt::info_hash_t& hash) const;

        std::size_t Count() const;

        auto Torrents() const { return m_torrents | std::views::filter(IsCurrent); }

        auto TorrentsAfter(const lt::info_hash_t& hash) const
        {
            return std::ranges::subrange(m_torrents.upper_bound(hash), m_torrents.end())
                | std::views::filter(IsCurrent);
        }

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

        std::string Log() const;
        std::string Log(const lt::info_hash_t& hash) const;

        // reconciles our torrents with the libtorrent session. use Jobs::ReconcileTorrents
        // to trigger this instead of calling directly
        void ReconcileTorrents();

        // removes any torrent handles that are invalid (is_valid()=false)
        void UntrackInvalidTorrents();

        // loads the next chunk of torrents
        bool LoadChunk(Data::Models::AddTorrentParams::Cursor& cursor, int limit, int& loaded);
        void LoadDone(int loaded, bool failed);

    private:
        void ReadAlerts();
        void ProcessAlert(const lt::alert* alert);

        void OnAddTorrentAlert(const lt::add_torrent_alert* alert);
        void OnFileErrorAlert(const lt::file_error_alert* alert);
        void OnSaveResumeDataAlert(const lt::save_resume_data_alert* alert);
        void OnTorrentRemovedAlert(const lt::torrent_removed_alert* alert);
        void OnTorrentResumedAlert(const lt::torrent_resumed_alert* alert);

        bool Track(const lt::torrent_status& status);

        // untracks a torrent from our end
        void UntrackTorrent(const lt::info_hash_t& hash);

        // whenever a torrent info hash is updated, for example due to metadata
        // turning it from a v1 into a hybrid
        void UpdateInfoHashes(const lt::info_hash_t& prev, const lt::info_hash_t& curr);

        template<typename T>
        void Publish(T event);

        template<typename Alert>
        void EmitTorrentEvent(std::string name, const Alert& alert, nlohmann::json extra = {});

        int            m_id;
        std::string    m_name;
        SessionOptions m_options;

        std::unique_ptr<lt::session>       m_session;
        std::map<lt::info_hash_t, Torrent> m_torrents;
        std::unique_ptr<Scheduler>         m_jobs;

        std::unordered_set<lt::info_hash_t> m_adding;
    };
}
