#include "../session.hpp"

#include <boost/log/trivial.hpp>

#include <libtorrent/alert_types.hpp>

#include "../publish.hpp"

#include "../../data/models/addtorrentparams.hpp"
#include "../../torrentclientdata.hpp"

using porla::Data::Models::AddTorrentParams;
using porla::Session;

void Session::OnAddTorrentAlert(const lt::add_torrent_alert* alert)
{
    const auto key = alert->params.ti
        ? alert->params.ti->info_hashes()
        : alert->params.info_hashes;

    if (alert->error)
    {
        BOOST_LOG_TRIVIAL(error) << Log(key) << "Failed to add torrent: " << alert->error.what();

        if (const auto it = m_torrents.find(key); it != m_torrents.end()
            && it->second.state != Torrent::State::Current)
        {
            m_torrents.erase(it);
        }

        return;
    }

    const auto hash = alert->handle.info_hashes();
          auto it   = m_torrents.find(key);

    if (it != m_torrents.end() && key != hash)
    {
        UpdateInfoHashes(key, hash);
        it = m_torrents.find(hash);
    }

    if (it == m_torrents.end())
    {
        BOOST_LOG_TRIVIAL(warning)
            << Log(hash) << "Received add torrent alert for a torrent Porla did not add";

        Track(alert->handle.status());

        it = m_torrents.find(hash);

        it->second.state = Torrent::State::Adding;
    }

    auto& torrent = it->second;

    switch (torrent.state)
    {
    case Torrent::State::Current:
        return;
    case Torrent::State::Loading:
    {
        // a torrent added from the sqlite db - promote it to current
        // but don't announce it to the world 
        torrent.state              = Torrent::State::Current;
        torrent.status.handle      = alert->handle;
        torrent.status.info_hashes = hash;

        return;
    }
    case Torrent::State::Adding:
        break;
    }

    const auto status = alert->handle.status();

    torrent.state  = Torrent::State::Current;
    torrent.status = status;

    const TorrentClientData fallback;

    try
    {
        AddTorrentParams::Insert(
            m_options.db,
            m_id,
            alert->handle.info_hashes(),
            alert->params,
            torrent.data == nullptr ? fallback : *torrent.data,
            static_cast<int>(status.queue_position));
    }
    catch(const std::exception& e)
    {
        BOOST_LOG_TRIVIAL(error)
            << Log(hash) << "Failed to insert: " << e.what();
    }

    alert->handle.save_resume_data(
        lt::torrent_handle::only_if_modified);

    EmitTorrentEvent("torrent.added", *alert);
}

void Session::OnTorrentRemovedAlert(const lt::torrent_removed_alert* alert)
{
    BOOST_LOG_TRIVIAL(info)
        << Log(alert->info_hashes) << "Torrent removed";

    UntrackTorrent(alert->info_hashes);
}

void Session::OnTorrentResumedAlert(const lt::torrent_resumed_alert* alert)
{
    BOOST_LOG_TRIVIAL(debug)
        << Log(alert->handle.info_hashes()) << "Torrent resumed";

    EmitTorrentEvent("torrent.resumed", *alert);
}

void Session::OnMetadataReceivedAlert(const lt::metadata_received_alert* alert)
{
    if (!alert->handle.is_valid())
    {
        return;
    }

    const auto info_hashes = alert->handle.info_hashes();

    BOOST_LOG_TRIVIAL(info) << Log(info_hashes) << "Metadata received";

    // A magnet link only carries the hash(es) it was added with. A hybrid
    // torrent gains its other hash with the metadata, and every later alert
    // uses the full hashes, so move the torrent to its new key.
    if (!m_torrents.contains(info_hashes))
    {
        const auto it = std::find_if(
            m_torrents.begin(),
            m_torrents.end(),
            [&](const auto& kv) { return kv.second.status.handle == alert->handle; });

        if (it != m_torrents.end())
        {
            UpdateInfoHashes(it->first, info_hashes);
        }
    }

    const auto it = m_torrents.find(info_hashes);

    MetadataReceived(alert->handle, it != m_torrents.end() ? &it->second : nullptr);
}

void Session::MetadataReceived(const lt::torrent_handle& th, Torrent* torrent)
{
    if (torrent == nullptr || !torrent->metadata_saved)
    {
        th.save_resume_data(lt::torrent_handle::save_info_dict);
    }

    if (torrent != nullptr && std::exchange(torrent->metadata_announced, true))
    {
        return;
    }

    if (m_options.events.HasSubscribers("torrent.metadata_received"))
    {
        TorrentEvent event("torrent.metadata_received", nlohmann::json::object());
        event.torrent_handle = th;
        event.info_hash      = th.info_hashes();

        Publish(std::move(event));
    }
}

void Session::OnTorrentErrorAlert(const lt::torrent_error_alert* alert)
{
    if (!alert->handle.is_valid())
    {
        return;
    }

    BOOST_LOG_TRIVIAL(error) << Log(alert->handle.info_hashes()) << alert->message();

    EmitTorrentEvent("torrent.error", *alert);
}

void Session::OnTorrentCheckedAlert(const lt::torrent_checked_alert* alert)
{
    if (!alert->handle.is_valid())
    {
        return;
    }

    BOOST_LOG_TRIVIAL(info) << Log(alert->handle.info_hashes())
        << "Torrent finished checking";

    if (const auto it = m_torrents.find(alert->handle.info_hashes());
        it != m_torrents.end() && it->second.restore_after_check.has_value())
    {
        const auto flags = std::exchange(it->second.restore_after_check, std::nullopt).value();

        if (flags & lt::torrent_flags::auto_managed)
        {
            alert->handle.set_flags(lt::torrent_flags::auto_managed);
        }

        if (flags & lt::torrent_flags::paused)
        {
            alert->handle.pause();
        }
    }

    EmitTorrentEvent("torrent.checked", *alert);
}

void Session::OnTorrentFinishedAlert(const lt::torrent_finished_alert* alert)
{
    if (!alert->handle.is_valid())
    {
        return;
    }

    const auto& status      = alert->handle.status();
          auto  client_data = alert->handle.userdata().get<TorrentClientData>();

    // libtorrent posts this on every transition into finished (startup checks, rechecks)
    // 'first' is true exactly once per torrent - the first time it finishes with payload
    // we downloaded.

    bool first = false;

    if (client_data != nullptr && !client_data->completed_at.has_value())
    {
        client_data->completed_at = std::time(nullptr);

        AddTorrentParams::UpdateClientData(
            m_options.db,
            m_id,
            status.info_hashes,
            *client_data);

        first = status.total_payload_download > 0;
    }

    if (first)
    {
        BOOST_LOG_TRIVIAL(info) << Log(status.info_hashes) << "Torrent finished";
    }

    status.handle.save_resume_data(
        lt::torrent_handle::only_if_modified);

    EmitTorrentEvent("torrent.finished", *alert, { { "first", first } });
}

void Session::OnTorrentPausedAlert(const lt::torrent_paused_alert* alert)
{
    BOOST_LOG_TRIVIAL(debug)
        << Log(alert->handle.info_hashes()) << "Torrent paused";

    EmitTorrentEvent("torrent.paused", *alert);
}

void Session::OnTrackerErrorAlert(const lt::tracker_error_alert* alert)
{
    if (alert->error == lt::errors::announce_skipped)
    {
        return;
    }

    EmitTorrentEvent("tracker.error", *alert);
}

