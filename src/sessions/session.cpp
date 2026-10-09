#include "session.hpp"

#include <boost/asio/post.hpp>
#include <boost/log/trivial.hpp>

#include <libtorrent/alert_types.hpp>
#include <libtorrent/extensions/smart_ban.hpp>
#include <libtorrent/extensions/ut_metadata.hpp>
#include <libtorrent/extensions/ut_pex.hpp>
#include <libtorrent/session_stats.hpp>

#include "jobs/loadtorrents.hpp"
#include "jobs/poststats.hpp"
#include "jobs/reconciletorrents.hpp"
#include "jobs/savestate.hpp"
#include "scheduler.hpp"
#include "sessionevent.hpp"
#include "torrent.hpp"
#include "torrentevent.hpp"

#include "../data/models/addtorrentparams.hpp"
#include "../data/transaction.hpp"
#include "../events.hpp"
#include "../json/all.hpp"
#include "../torrentclientdata.hpp"
#include "../utils/hex.hpp"

using porla::Data::Models::AddTorrentParams;
using porla::Data::Transaction;
using porla::Session;

namespace
{
    const std::vector<lt::stats_metric> kSessionMetrics = lt::session_stats_metrics();
}

Session::Session(const SessionOptions& options)
    : m_id(options.record.id)
    , m_name(options.record.name)
    , m_options(options)
    , m_jobs(std::make_unique<Scheduler>(options.io, *this))
{
    m_session = std::make_unique<lt::session>(std::move(m_options.record.params));
    m_session->add_extension(&lt::create_smart_ban_plugin);
    m_session->add_extension(&lt::create_ut_metadata_plugin);
    m_session->add_extension(&lt::create_ut_pex_plugin);
}

Session::~Session()
{
    if (m_session == nullptr)
    {
        return;
    }

    std::vector<TorrentClientData*> untracked;

    try
    {
        for (const auto& th : m_session->get_torrents())
        {
            if (m_torrents.contains(th.info_hashes()))
            {
                continue;
            }

            if (auto* client_data = th.userdata().get<TorrentClientData>())
            {
                untracked.push_back(client_data);
            }
        }
    }
    catch(const std::exception& e)
    {
        BOOST_LOG_TRIVIAL(error) << Log() << "Failed to list torrents on teardown: " << e.what();
    }

    m_session.reset();

    for (auto* d : untracked)
    {
        delete d;
    }
}

void Session::Start(std::function<void()> load_callback)
{
    m_session->set_alert_notify(
        [io = &m_options.io, t = weak_from_this()]()
        {
            boost::asio::post(*io, [t]
            {
                if (auto self = t.lock()) { self->ReadAlerts(); }
            });
        });

    using ms = std::chrono::milliseconds;

    m_jobs->Every(ms(m_options.record.timer_dht_stats),       std::make_unique<Jobs::PostDhtStats>());
    m_jobs->Every(ms(m_options.record.timer_save_state),      std::make_unique<Jobs::SaveState>());
    m_jobs->Every(ms(m_options.record.timer_session_stats),   std::make_unique<Jobs::PostSessionStats>());
    m_jobs->Every(ms(m_options.record.timer_torrent_updates), std::make_unique<Jobs::PostTorrentUpdates>());

    m_jobs->Register(std::make_unique<Jobs::ReconcileTorrents>());

    // suspend manual jobs (such as reconciliation) until load is done
    m_jobs->Suspend();

    const auto count = AddTorrentParams::Count(m_options.db, m_options.record.id);

    BOOST_LOG_TRIVIAL(info)
        << Log() << "Loading " << count << " torrent(s) from storage";

    m_jobs->Start(std::make_unique<Jobs::LoadTorrents>(count, std::move(load_callback)));
}

const porla::Torrent* Session::Find(const lt::info_hash_t& hash) const
{
    const auto it = m_torrents.find(hash);

    if (it == m_torrents.end() || it->second.state != Torrent::State::Current)
    {
        return nullptr;
    }

    return &it->second;
}

std::size_t Session::Count() const
{
    return static_cast<std::size_t>(std::ranges::distance(Torrents()));
}

void Session::Stop()
{
    m_jobs->Stop();

    // reconcile torrents whose add alert was lost, otherwise they
    // will be gone
    ReconcileTorrents();

    m_session->set_alert_notify({});

    Persist();

    m_session->pause();

    auto torrents = m_session->get_torrent_status(
        [](const lt::torrent_status& ts)
        {
            return ts.has_metadata && bool(ts.need_save_resume_data);
        });

    int chunk_size = 1000;
    int chunks     = static_cast<int>(torrents.size() / chunk_size) + 1;

    BOOST_LOG_TRIVIAL(info)
        << Log() << "Saving resume data in " << chunks
        << " chunk(s) - total torrents: " << torrents.size();

    auto current = torrents.begin();

    for (int i = 0; i < chunks; i++)
    {
        int chunk_items = std::min(
            chunk_size,
            static_cast<int>(std::distance(current, torrents.end())));

        int outstanding = 0;

        for (int j = 0; j < chunk_items; j++)
        {
            if (!current->handle.is_valid())
            {
                std::advance(current, 1);
                continue;
            }

            try
            {
                current->handle.save_resume_data(
                    lt::torrent_handle::flush_disk_cache
                    | lt::torrent_handle::only_if_modified);

                outstanding++;
            }
            catch(const std::exception& e)
            {
                BOOST_LOG_TRIVIAL(warning)
                    << Log() << "Failed to post save resume data: " << e.what();
            }

            std::advance(current, 1);
        }

        BOOST_LOG_TRIVIAL(info)
            << Log() << "Chunk " << i + 1 << " - Saving state for "
            << outstanding << " torrent(s) (out of " << chunk_items << ")";

        while (outstanding > 0) {
            const auto found_alert = m_session->wait_for_alert(lt::seconds(10));

            if (!found_alert)
            {
                BOOST_LOG_TRIVIAL(warning)
                    << Log() << "Gave up waiting for " << outstanding << " resume data alerts "
                    << "in chunk " << i + 1;

                break;
            }

            std::vector<lt::alert *> alerts;
            m_session->pop_alerts(&alerts);

            const Transaction tx(m_options.db);

            for (lt::alert *a: alerts)
            {
                if (lt::alert_cast<lt::torrent_paused_alert>(a))
                {
                    continue;
                }

                if (auto fail = lt::alert_cast<lt::save_resume_data_failed_alert>(a))
                {
                    outstanding--;

                    BOOST_LOG_TRIVIAL(error)
                        << Log(fail->handle.info_hashes())
                        << "Failed to save resume data: " << fail->message();

                    continue;
                }

                if (auto removed = lt::alert_cast<lt::torrent_removed_alert>(a))
                {
                    OnTorrentRemovedAlert(removed);
                    continue;
                }

                const auto rd = lt::alert_cast<lt::save_resume_data_alert>(a);
                if (!rd) { continue; }

                outstanding--;

                if (!rd->handle.is_valid())
                {
                    continue;
                }

                try
                {
                    const auto data      = rd->handle.userdata().get<TorrentClientData>();
                    const auto info_hash = rd->handle.info_hashes();

                    AddTorrentParams::Update(
                        m_options.db,
                        m_id,
                        info_hash,
                        rd->params,
                        data,
                        static_cast<int>(rd->handle.queue_position()));
                }
                catch(const std::exception& e)
                {
                    BOOST_LOG_TRIVIAL(error)
                        << Log(rd->params.info_hashes) << "Failed to save resume data: " << e.what();
                }
            }
        }
    }
}

void Session::Persist()
{
    Data::Models::Sessions::UpdateParams(
        m_options.db,
        m_id,
        m_session->session_state());
}

void Session::Recheck(const lt::info_hash_t& hash)
{
    const auto it = m_torrents.find(hash);

    if (it == m_torrents.end())
    {
        BOOST_LOG_TRIVIAL(warning)
            << Log(hash) << "Failed to find torrent for rechecking";

        return;
    }

          auto& torrent = it->second;
    const auto& handle  = torrent.status.handle;

    // libtorrent does not check a paused torrent, and auto management could pause it again.
    // Run it unmanaged for the check, and put the flags back when torrent_checked arrives.
    const auto flags = handle.flags() & (lt::torrent_flags::paused | lt::torrent_flags::auto_managed);

    if (flags)
    {
        torrent.restore_after_check = flags;

        handle.unset_flags(lt::torrent_flags::auto_managed);
        handle.resume();
    }

    handle.force_recheck();
}

bool Session::LoadChunk(Data::Models::AddTorrentParams::Cursor& cursor, int limit, int& loaded)
{
    // drain alert queue between chunks
    try
    {   
        ReadAlerts();
    }
    catch(const std::exception& e)
    {
        BOOST_LOG_TRIVIAL(error) << Log() << "Failed to read alerts during load: " << e.what();
    }

    return AddTorrentParams::Next(
        m_options.db,
        m_id,
        cursor,
        limit,
        [&](lt::add_torrent_params& params)
        {
            if (m_adding.contains(params.info_hashes)
                || m_torrents.contains(params.info_hashes))
            {
                // never handed to libtorrent, so nothing else frees it
                delete params.userdata.get<TorrentClientData>();
                return;
            }

            params.userdata.get<TorrentClientData>()->session = weak_from_this();

            m_adding.insert(params.info_hashes);
            m_session->async_add_torrent(params);

            loaded++;
        });
}

void Session::LoadDone(int loaded, bool failed)
{
    if (loaded > 0)
    {
        try
        {
            const auto& all_statuses = m_session->get_torrent_status(
                [](const auto& ts) { return true; },
                lt::status_flags_t::all());

            for (const auto& ts : all_statuses)
            {
                if (const auto it = m_torrents.find(ts.info_hashes); it != m_torrents.end())
                {
                    it->second.status = ts;
                }
                else if (m_adding.contains(ts.info_hashes))
                {
                    Track(ts);
                }
            }

            m_adding.clear();
        }
        catch (const std::exception& e)
        {
            BOOST_LOG_TRIVIAL(error)
                << Log() << "Failed to read torrent status after load: " << e.what();
        }
    }

    Publish(SessionEvent("session.loaded", {
        { "torrents", loaded },
        { "failed",   failed }
    }));

    m_jobs->Resume();
}

void Session::ReadAlerts()
{
    std::vector<lt::alert*> alerts;
    m_session->pop_alerts(&alerts);

    if (alerts.empty())
    {
        return;
    }

    const Transaction tx(m_options.db);

    for (const auto alert : alerts)
    {
        try
        {
            ProcessAlert(alert);
        }
        catch(const std::exception& e)
        {
            BOOST_LOG_TRIVIAL(error)
                << Log() << "Failed to process alert " << alert->what() << ": " << e.what();
        }
    }
}

void Session::ProcessAlert(const lt::alert* alert)
{
    BOOST_LOG_TRIVIAL(trace) << Log() << alert->what() << ": " << alert->message();

    switch (alert->type())
    {
        case lt::add_torrent_alert::alert_type:      OnAddTorrentAlert(lt::alert_cast<lt::add_torrent_alert>(alert));          break;
        case lt::file_error_alert::alert_type:       OnFileErrorAlert(lt::alert_cast<lt::file_error_alert>(alert));            break;
        case lt::save_resume_data_alert::alert_type: OnSaveResumeDataAlert(lt::alert_cast<lt::save_resume_data_alert>(alert)); break;
        case lt::torrent_removed_alert::alert_type:  OnTorrentRemovedAlert(lt::alert_cast<lt::torrent_removed_alert>(alert));  break;
        case lt::torrent_resumed_alert::alert_type:  OnTorrentResumedAlert(lt::alert_cast<lt::torrent_resumed_alert>(alert));  break;

        case lt::alerts_dropped_alert::alert_type:
        {
            const auto ada = lt::alert_cast<lt::alerts_dropped_alert>(alert);

            std::string types;

            for (int i = 0; i < lt::num_alert_types; i++)
            {
                if (ada->dropped_alerts.test(i))
                {
                    types += (types.empty() ? "" : ",") + std::string(lt::alert_name(i));
                }
            }

            BOOST_LOG_TRIVIAL(warning)
                << Log() << "The libtorrent alert queue is full and dropped "
                << types << " alerts. Consider raising alert_queue_size.";

            if (ada->dropped_alerts.test(lt::add_torrent_alert::alert_type)
                || ada->dropped_alerts.test(lt::torrent_removed_alert::alert_type))
            {
                m_jobs->Trigger<Jobs::ReconcileTorrents>();
            }

            break;
        }

        case lt::listen_failed_alert::alert_type:
        {
            const auto lfa = lt::alert_cast<lt::listen_failed_alert>(alert);
            BOOST_LOG_TRIVIAL(warning) << Log() << lfa->message();
            break;
        }
        case lt::listen_succeeded_alert::alert_type:
        {
            const auto lsa = lt::alert_cast<lt::listen_succeeded_alert>(alert);
            BOOST_LOG_TRIVIAL(info) << Log() << lsa->message();
            break;
        }
        case lt::metadata_received_alert::alert_type:
        {
            auto mra = lt::alert_cast<lt::metadata_received_alert>(alert);

            if (!mra->handle.is_valid())
            {
                break;
            }

            const auto info_hashes = mra->handle.info_hashes();

            BOOST_LOG_TRIVIAL(info) << Log(info_hashes) << "Metadata received";

            // A magnet link only carries the hash(es) it was added with. A hybrid
            // torrent gains its other hash with the metadata, and every later alert
            // uses the full hashes, so move the torrent to its new key.
            if (!m_torrents.contains(info_hashes))
            {
                const auto it = std::find_if(
                    m_torrents.begin(),
                    m_torrents.end(),
                    [&](const auto& kv) { return kv.second.status.handle == mra->handle; });

                if (it != m_torrents.end())
                {
                    UpdateInfoHashes(it->first, info_hashes);
                }
            }

            mra->handle.save_resume_data(
                lt::torrent_handle::save_info_dict);

            EmitTorrentEvent("torrent.metadata_received", *mra);

            break;
        }
        case lt::session_stats_alert::alert_type:
        {
            if (!m_options.events.HasSubscribers("session.stats"))
            {
                break;
            }

            const auto counters = lt::alert_cast<lt::session_stats_alert>(alert)->counters();

            nlohmann::json stats = nlohmann::json::object();

            for (const auto& m : kSessionMetrics)
            {
                stats[m.name] = counters[m.value_index];
            }

            Publish(SessionEvent("session.stats", {
                { "stats", std::move(stats) }
            }));

            break;
        }
        case lt::state_update_alert::alert_type:
        {
            auto sua = lt::alert_cast<lt::state_update_alert>(alert);

            for (const auto& status : sua->status)
            {
                const auto it = m_torrents.find(status.info_hashes);

                if (it == m_torrents.end())
                {
                    BOOST_LOG_TRIVIAL(debug)
                        << Log(status.info_hashes) << "Received state update for non-tracked torrent";

                    m_jobs->Trigger<Jobs::ReconcileTorrents>();

                    continue;
                }

                it->second.status = status;
            }

            break;
        }
        case lt::storage_moved_alert::alert_type:
        {
            const auto sma = lt::alert_cast<lt::storage_moved_alert>(alert);

            if (!sma->handle.is_valid())
            {
                break;
            }

            BOOST_LOG_TRIVIAL(info)
                << Log(sma->handle.info_hashes()) << "Storage moved to " << sma->storage_path();

            sma->handle.save_resume_data(
                lt::torrent_handle::only_if_modified);

            sma->handle.post_status();

            EmitTorrentEvent("torrent.moved", *sma);

            break;
        }
        case lt::storage_moved_failed_alert::alert_type:
        {
            const auto smfa = lt::alert_cast<lt::storage_moved_failed_alert>(alert);

            if (!smfa->handle.is_valid())
            {
                break;
            }

            BOOST_LOG_TRIVIAL(warning) << Log(smfa->handle.info_hashes()) << smfa->message();

            EmitTorrentEvent("torrent.move_failed", *smfa);

            break;
        }
        case lt::state_changed_alert::alert_type:
        {
            const auto sca = lt::alert_cast<lt::state_changed_alert>(alert);

            EmitTorrentEvent("torrent.state_changed", *sca);

            break;
        }
        case lt::torrent_error_alert::alert_type:
        {
            const auto tea = lt::alert_cast<lt::torrent_error_alert>(alert);

            if (!tea->handle.is_valid())
            {
                break;
            }

            BOOST_LOG_TRIVIAL(error) << Log(tea->handle.info_hashes()) << tea->message();

            EmitTorrentEvent("torrent.error", *tea);

            break;
        }
        case lt::torrent_checked_alert::alert_type:
        {
            const auto tca = lt::alert_cast<lt::torrent_checked_alert>(alert);

            if (!tca->handle.is_valid())
            {
                break;
            }

            BOOST_LOG_TRIVIAL(info) << Log(tca->handle.info_hashes())
                << "Torrent finished checking";

            if (const auto it = m_torrents.find(tca->handle.info_hashes());
                it != m_torrents.end() && it->second.restore_after_check.has_value())
            {
                const auto flags = std::exchange(it->second.restore_after_check, std::nullopt).value();

                // TODO: Unsure about the order here. If there are reports that force-checking a torrent
                //       leads to any issues with resume/pause, the order of these statements might matter.

                if (flags & lt::torrent_flags::auto_managed)
                {
                    tca->handle.set_flags(lt::torrent_flags::auto_managed);
                }

                if (flags & lt::torrent_flags::paused)
                {
                    tca->handle.pause();
                }
            }

            EmitTorrentEvent("torrent.checked", *tca);

            break;
        }
        case lt::torrent_finished_alert::alert_type:
        {
            const auto tfa = lt::alert_cast<lt::torrent_finished_alert>(alert);

            if (!tfa->handle.is_valid())
            {
                break;
            }

            const auto& status      = tfa->handle.status();
                  auto  client_data = tfa->handle.userdata().get<TorrentClientData>();

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

            EmitTorrentEvent("torrent.finished", *tfa, { { "first", first } });

            break;
        }
        case lt::torrent_paused_alert::alert_type:
        {
            const auto tpa = lt::alert_cast<lt::torrent_paused_alert>(alert);

            BOOST_LOG_TRIVIAL(debug)
                << Log(tpa->handle.info_hashes()) << "Torrent paused";

            EmitTorrentEvent("torrent.paused", *tpa);

            break;
        }
        case lt::tracker_error_alert::alert_type:
        {
            const auto tea = lt::alert_cast<lt::tracker_error_alert>(alert);

            if (tea->error == lt::errors::announce_skipped)
            {
                break;
            }

            EmitTorrentEvent("tracker.error", *tea);

            break;
        }
        case lt::tracker_reply_alert::alert_type:
        {
            const auto tra = lt::alert_cast<lt::tracker_reply_alert>(alert);

            EmitTorrentEvent("tracker.reply", *tra);

            break;
        }
        case lt::tracker_warning_alert::alert_type:
        {
            const auto twa = lt::alert_cast<lt::tracker_warning_alert>(alert);

            EmitTorrentEvent("tracker.warning", *twa);

            break;
        }
    }
}

void Session::OnAddTorrentAlert(const lt::add_torrent_alert* alert)
{
    if (alert->error)
    {
        const auto name = alert->params.ti
            ? alert->params.ti->name()
            : alert->params.name;

        const auto info_hash = alert->params.ti
            ? alert->params.ti->info_hashes()
            : alert->params.info_hashes;

        BOOST_LOG_TRIVIAL(error)
            << Log(info_hash)
            << "Failed to add torrent: " << alert->error.what();

        m_adding.erase(info_hash);

        delete alert->params.userdata.get<TorrentClientData>();

        return;
    }

    // this torrent was part of the load - we store it in our map
    // but we don't announce it
    if (m_adding.erase(alert->handle.info_hashes()) > 0)
    {
        lt::torrent_status status;
        status.handle      = alert->handle;
        status.info_hashes = alert->handle.info_hashes();

        Track(status);

        return;
    }

    const auto data   = alert->handle.userdata().get<TorrentClientData>();
    const auto status = alert->handle.status();

    if (!Track(status))
    {
        BOOST_LOG_TRIVIAL(debug)
            << Log(status.info_hashes)
            << "Torrent already in session - ignoring duplicate add";

        auto* extra = alert->params.userdata.get<TorrentClientData>();

        if (extra != nullptr && extra != data)
        {
            delete extra;
        }

        return;
    }

    if (data == nullptr)
    {
        BOOST_LOG_TRIVIAL(error)
            << Log(status.info_hashes) << "Missing userdata for torrent";
    }

    const TorrentClientData fallback;

    try
    {
        AddTorrentParams::Insert(
            m_options.db,
            m_id,
            alert->handle.info_hashes(),
            alert->params,
            data == nullptr ? fallback : *data,
            static_cast<int>(status.queue_position));
    }
    catch(const std::exception& e)
    {
        BOOST_LOG_TRIVIAL(error)
            << Log(status.info_hashes) << "Failed to insert: " << e.what();
    }

    alert->handle.save_resume_data(
        lt::torrent_handle::only_if_modified);

    EmitTorrentEvent("torrent.added", *alert);
}

void Session::OnFileErrorAlert(const lt::file_error_alert* alert)
{
    EmitTorrentEvent("torrent.file_error", *alert);
}

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

    AddTorrentParams::Update(
        m_options.db,
        m_id,
        info_hashes,
        alert->params,
        data,
        static_cast<int>(alert->handle.queue_position()));

    BOOST_LOG_TRIVIAL(debug) << Log(info_hashes) << "Resume data saved";
}

void Session::OnTorrentRemovedAlert(const lt::torrent_removed_alert* alert)
{
    BOOST_LOG_TRIVIAL(info)
        << Log(alert->info_hashes) << "Torrent removed";

    const bool loading = m_adding.contains(alert->info_hashes);

    UntrackTorrent(alert->info_hashes);

    if (loading)
    {
        delete alert->userdata.get<TorrentClientData>();
    }
}

void Session::OnTorrentResumedAlert(const lt::torrent_resumed_alert* alert)
{
    BOOST_LOG_TRIVIAL(debug)
        << Log(alert->handle.info_hashes()) << "Torrent resumed";

    EmitTorrentEvent("torrent.resumed", *alert);
}

void Session::ReconcileTorrents()
{
    std::vector<lt::torrent_handle> handles;

    try
    {
        handles = m_session->get_torrents();
    }
    catch(const std::exception& e)
    {
        BOOST_LOG_TRIVIAL(error)
            << Log() << "Failed to list torrents to reconcile: " << e.what();

        return;
    }

    std::unordered_set<lt::info_hash_t>                          live;
    std::optional<std::map<lt::torrent_handle, lt::info_hash_t>> keys;

    int tracked   = 0;
    int loaded    = 0;
    int rehashed  = 0;
    int untracked = 0;

    const Transaction tx(m_options.db);

    for (const auto& th : handles)
    {
        const auto hash = th.info_hashes();

        live.insert(hash);

        if (m_torrents.contains(hash))
        {
            continue;
        }

        if (!keys)
        {
            keys.emplace();

            for (const auto& [ h, s ] : m_torrents)
            {
                keys->emplace(s.status.handle, h);
            }
        }

        if (const auto key = keys->find(th); key != keys->end())
        {
            UpdateInfoHashes(key->second, hash);

            try
            {
                const auto params = th.get_resume_data(lt::torrent_handle::save_info_dict);

                AddTorrentParams::Update(
                    m_options.db,
                    m_id,
                    hash,
                    params,
                    th.userdata().get<TorrentClientData>(),
                    static_cast<int>(th.queue_position()));
            }
            catch(const std::exception& e)
            {
                BOOST_LOG_TRIVIAL(error) << Log(hash) << "Failed to store torrent with updated hashes: " << e.what();
            }

            rehashed++;

            continue;
        }

        const auto status = th.status();

        Track(status);

        if (m_adding.erase(hash) > 0)
        {
            loaded++;
            continue;
        }

        const auto* data = th.userdata().get<TorrentClientData>();

        try
        {
            const auto params = th.get_resume_data(lt::torrent_handle::save_info_dict);

            AddTorrentParams::Insert(
                m_options.db,
                m_id,
                hash,
                params,
                data == nullptr ? TorrentClientData{} : *data,
                static_cast<int>(status.queue_position));
        }
        catch (const std::exception& e)
        {
            BOOST_LOG_TRIVIAL(error)
                << Log(hash) << "Failed to insert reconciled torrent: " << e.what();
        }

        if (m_options.events.HasSubscribers("torrent.added"))
        {
            nlohmann::json event_data = nlohmann::json::object();
            event_data["error"] = lt::error_code{};

            TorrentEvent added("torrent.added", std::move(event_data));
            added.torrent_handle = th;
            added.info_hash      = hash;

            Publish(std::move(added));
        }

        tracked++;
    }

    if (m_torrents.size() != handles.size())
    {
        std::vector<lt::info_hash_t> gone;

        for (const auto& [ hash, _ ] : m_torrents)
        {
            if (!live.contains(hash))
            {
                gone.push_back(hash);
            }
        }

        for (const auto& hash : gone)
        {
            UntrackTorrent(hash);
            untracked++;
        }
    }

    if (tracked + loaded + rehashed + untracked == 0)
    {
        return;
    }

    BOOST_LOG_TRIVIAL(warning)
        << Log() << "Reconciled torrents:"
        << " tracked=" << tracked
        << " loaded=" << loaded
        << " rehashed=" << rehashed
        << " untracked=" << untracked;
}

bool Session::Track(const lt::torrent_status& status)
{
    const auto [ it, inserted ] = m_torrents.try_emplace(status.info_hashes);

    it->second.state  = Torrent::State::Current;
    it->second.status = status;

    if (inserted)
    {
        it->second.data.reset(status.handle.userdata().get<TorrentClientData>());
    }

    return inserted;
}

void Session::UntrackInvalidTorrents()
{
    std::vector<lt::info_hash_t> gone;

    for (const auto& [ hash, t ] : m_torrents)
    {
        if (!t.status.handle.is_valid())
        {
            gone.push_back(hash);
        }
    }

    if (gone.empty())
    {
        return;
    }

    for (const auto& hash : gone)
    {
        UntrackTorrent(hash);
    }

    BOOST_LOG_TRIVIAL(warning)
        << Log() << "Removed " << gone.size() << " orphaned torrent(s). "
        << "This might be due to dropped alerts. Consider increasing alert_queue_size";
}

void Session::UntrackTorrent(const lt::info_hash_t& hash)
{
    m_adding.erase(hash);

    const bool tracked = m_torrents.erase(hash) > 0;

    if (tracked)
    {
        TorrentEvent removed("torrent.removed");
        removed.info_hash = hash;

        Publish(std::move(removed));
    }

    try
    {
        AddTorrentParams::Remove(m_options.db, m_id, hash);
    }
    catch(const std::exception& e)
    {
        BOOST_LOG_TRIVIAL(error)
            << Log(hash) << "Failed to remove torrent from database: "
            << e.what();
    }
}

void Session::UpdateInfoHashes(const lt::info_hash_t& prev, const lt::info_hash_t& curr)
{
    auto node = m_torrents.extract(prev);

    if (node.empty())
    {
        return;
    }

    node.key() = curr;
    node.mapped().status.info_hashes = curr;

    m_torrents.insert(std::move(node));
}

template<typename T>
void Session::Publish(T event)
{
    static_assert(std::is_base_of_v<SessionEvent, T>);

    event.session_id = m_id;
    event.session    = weak_from_this();

    m_options.events.Publish(std::move(event));
}

template<typename Alert>
void Session::EmitTorrentEvent(std::string name, const Alert& alert, nlohmann::json extra)
{
    if (!m_options.events.HasSubscribers(name) || !alert.handle.is_valid())
    {
        return;
    }

    nlohmann::json data = alert;

    if (extra.is_object())
    {
        data.update(extra);
    }

    TorrentEvent event(std::move(name), std::move(data));
    event.torrent_handle = alert.handle;
    event.info_hash      = alert.handle.info_hashes();

    Publish(std::move(event));
}

std::string Session::Log() const
{
    return "session[" + m_name + "] ";
}

std::string Session::Log(const lt::info_hash_t& hash) const
{
    const auto hash_hex = hash.has_v1()
        ? porla::Utils::ToHex({ hash.v1.data(), static_cast<std::size_t>(hash.v1.size()) })
        : porla::Utils::ToHex({ hash.v2.data(), static_cast<std::size_t>(hash.v2.size()) });

    return "session[" + m_name + "][" + hash_hex.substr(0, 8) + "] ";
}
