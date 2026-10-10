#include "../session.hpp"

#include <boost/log/trivial.hpp>

#include <libtorrent/alert_types.hpp>

#include "../publish.hpp"

#include "../../data/models/addtorrentparams.hpp"
#include "../../torrentclientdata.hpp"

using porla::Data::Models::AddTorrentParams;
using porla::Session;

void Session::OnSaveResumeDataAlert(const lt::save_resume_data_alert* alert)
{
    if (!alert->handle.is_valid())
    {
        BOOST_LOG_TRIVIAL(debug)
            << Log(alert->params.info_hashes) << "Received resume data for invalid torrent";

        return;
    }

    const auto data        = alert->handle.userdata().get<TorrentClientData>();
    const auto info_hashes = alert->handle.info_hashes();

    try
    {
        AddTorrentParams::Update(
            m_options.db,
            m_id,
            info_hashes,
            alert->params,
            data,
            static_cast<int>(alert->handle.queue_position()));
    }
    catch(const std::exception& e)
    {
        BOOST_LOG_TRIVIAL(error)
            << Log(info_hashes) << "Failed to save resume data: " << e.what();

        return;
    }

    if (alert->params.ti)
    {
        if (const auto it = m_torrents.find(info_hashes); it != m_torrents.end())
        {
            it->second.metadata_saved = true;
        }
    }

    BOOST_LOG_TRIVIAL(debug) << Log(info_hashes) << "Resume data saved";
}

void Session::OnStorageMovedAlert(const lt::storage_moved_alert* alert)
{
    if (!alert->handle.is_valid())
    {
        return;
    }

    BOOST_LOG_TRIVIAL(info)
        << Log(alert->handle.info_hashes()) << "Storage moved to " << alert->storage_path();

    alert->handle.save_resume_data(
        lt::torrent_handle::only_if_modified);

    alert->handle.post_status();

    EmitTorrentEvent("torrent.moved", *alert);
}

void Session::OnStorageMovedFailedAlert(const lt::storage_moved_failed_alert* alert)
{
    if (!alert->handle.is_valid())
    {
        return;
    }

    BOOST_LOG_TRIVIAL(warning) << Log(alert->handle.info_hashes()) << alert->message();

    EmitTorrentEvent("torrent.move_failed", *alert);
}
