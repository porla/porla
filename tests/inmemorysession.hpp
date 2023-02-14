#pragma once

#include "../src/session.hpp"

class InMemorySession : public porla::ISession
{
public:
    boost::signals2::connection OnSessionStats(const SessionStatsSignal::slot_type& subscriber) override
    {
        return m_sessionStats.connect(subscriber);
    }

    boost::signals2::connection OnStateUpdate(const TorrentStatusListSignal::slot_type& subscriber) override
    {
        return m_stateUpdate.connect(subscriber);
    }

    boost::signals2::connection OnStorageMoved(const TorrentHandleSignal::slot_type& subscriber) override
    {
        return m_storageMoved.connect(subscriber);
    }

    boost::signals2::connection OnStorageMovedFailed(const TorrentHandleSignal::slot_type& subscriber) override
    {
        return m_storageMovedFailed.connect(subscriber);
    }

    boost::signals2::connection OnTorrentAdded(const TorrentStatusSignal::slot_type& subscriber) override
    {
        return m_torrentAdded.connect(subscriber);
    }

    boost::signals2::connection OnTorrentFinished(const TorrentStatusSignal::slot_type& subscriber) override
    {
        return m_torrentFinished.connect(subscriber);
    }

    boost::signals2::connection OnTorrentMediaInfo(const TorrentHandleSignal::slot_type& subscriber) override
    {
        return m_torrentMediaInfo.connect(subscriber);
    }

    boost::signals2::connection OnTorrentPaused(const TorrentHandleSignal::slot_type& subscriber) override
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

    boost::signals2::connection OnTorrentTrackerError(const TrackerErrorSignal::slot_type& subscriber) override
    {
        return m_torrentTrackerError.connect(subscriber);
    }

    boost::signals2::connection OnTorrentTrackerReply(const TorrentHandleSignal::slot_type& subscriber) override
    {
        return m_torrentTrackerReply.connect(subscriber);
    }

    libtorrent::info_hash_t AddTorrent(libtorrent::add_torrent_params const& p) override;
    void ApplySettings(const libtorrent::settings_pack& settings) override;
    void Pause() override;
    void Recheck(const lt::info_hash_t& hash) override;
    void Remove(const lt::info_hash_t& hash, bool remove_data) override;
    void Resume() override;
    libtorrent::settings_pack Settings() override;
    const std::map<lt::info_hash_t, lt::torrent_handle>& Torrents() override;

    SessionStatsSignal m_sessionStats;
    TorrentStatusListSignal m_stateUpdate;
    TorrentHandleSignal m_storageMoved;
    TorrentHandleSignal m_storageMovedFailed;
    TorrentStatusSignal m_torrentAdded;
    TorrentStatusSignal m_torrentFinished;
    TorrentHandleSignal m_torrentMediaInfo;
    TorrentHandleSignal m_torrentPaused;
    InfoHashSignal m_torrentRemoved;
    TorrentStatusSignal m_torrentResumed;
    TrackerErrorSignal m_torrentTrackerError;
    TorrentHandleSignal m_torrentTrackerReply;

    std::map<lt::info_hash_t, lt::torrent_handle> m_torrents;
};
