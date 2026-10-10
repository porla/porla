#include "session.hpp"

#include <algorithm>

#include <boost/asio/post.hpp>
#include <boost/log/trivial.hpp>

#include <libtorrent/alert_types.hpp>
#include <libtorrent/extensions/smart_ban.hpp>
#include <libtorrent/extensions/ut_metadata.hpp>
#include <libtorrent/extensions/ut_pex.hpp>

#include "jobs/loadtorrents.hpp"
#include "jobs/poststats.hpp"
#include "jobs/reconciletorrents.hpp"
#include "jobs/savestate.hpp"
#include "alertdispatcher.hpp"
#include "publish.hpp"
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

Session::Session(const SessionOptions& options)
    : m_id(options.record.id)
    , m_name(options.record.name)
    , m_options(options)
    , m_alert_dispatcher(std::make_unique<AlertDispatcher>())
    , m_jobs(std::make_unique<Scheduler>(options.io, *this))
{
    m_session = std::make_unique<lt::session>(std::move(m_options.record.params));
    m_session->add_extension(&lt::create_smart_ban_plugin);
    m_session->add_extension(&lt::create_ut_metadata_plugin);
    m_session->add_extension(&lt::create_ut_pex_plugin);

    RegisterAlertHandlers();
}

Session::~Session()
{
    // bye bye libtorrent session
    m_session.reset();
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

    m_jobs->Start(std::make_unique<Jobs::LoadTorrents>(
        m_options.db,
        m_options.events,
        *m_jobs,
        std::move(load_callback)));
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

bool Session::Add(lt::add_torrent_params params, Torrent::State state)
{
    // take ownership of the userdata immediately
    std::unique_ptr<TorrentClientData> data(params.userdata.get<TorrentClientData>());

    const auto hash = params.ti
        ? params.ti->info_hashes()
        : params.info_hashes;

    const auto [ it, inserted ] = m_torrents.try_emplace(hash);

    if (!inserted)
    {
        return false;
    }

    if (data != nullptr)
    {
        data->session = weak_from_this();
    }

    it->second.state              = state;
    it->second.data               = std::move(data);
    it->second.metadata_announced = params.ti != nullptr;
    it->second.metadata_saved     = params.ti != nullptr;

    // always set the update_subscribe flag so we are always getting
    // the update alerts
    params.flags |= lt::torrent_flags::update_subscribe;

    try
    {
        m_session->async_add_torrent(std::move(params));
    }
    catch(...)
    {
        m_torrents.erase(it);
        throw;
    }

    return true;
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
        [](const lt::torrent_status& ts) { return ts.has_metadata; });

    const auto missing_ti = [&](const lt::torrent_status& ts)
    {
        const auto it = m_torrents.find(ts.info_hashes);
        return it != m_torrents.end() && !it->second.metadata_saved;
    };

    std::erase_if(
        torrents,
        [&](const lt::torrent_status& ts)
        {
            return !ts.need_save_resume_data && !missing_ti(ts);
        });

    const auto current_settings = m_session->get_settings();
    const int  alert_queue_size = current_settings.get_int(lt::settings_pack::alert_queue_size);
    const int  chunk_size       = std::clamp(alert_queue_size, 1, 1000);
    const int  chunks           = static_cast<int>(torrents.size() / chunk_size) + 1;

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

            const auto flags = missing_ti(*current)
                ? lt::torrent_handle::flush_disk_cache | lt::torrent_handle::save_info_dict
                : lt::torrent_handle::flush_disk_cache | lt::torrent_handle::only_if_modified;

            try
            {
                current->handle.save_resume_data(flags);
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
                if (auto fail = lt::alert_cast<lt::save_resume_data_failed_alert>(a))
                {
                    outstanding--;

                    BOOST_LOG_TRIVIAL(error)
                        << Log(fail->handle.info_hashes())
                        << "Failed to save resume data: " << fail->message();

                    continue;
                }

                if (lt::alert_cast<lt::save_resume_data_alert>(a))
                {
                    outstanding--;
                    m_alert_dispatcher->Dispatch(a);
                    continue;
                }

                if (lt::alert_cast<lt::torrent_removed_alert>(a))
                {
                    m_alert_dispatcher->Dispatch(a);
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

    if (it == m_torrents.end() || it->second.state != Torrent::State::Current)
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
    m_alert_dispatcher->Dispatch(alert);
}

template<typename A>
void Session::Forward(std::string name)
{
    m_alert_dispatcher->On<A>(
        [this, name = std::move(name)](const A* alert)
        {
            EmitTorrentEvent(name, *alert);
        });
}

void Session::RegisterAlertHandlers()
{
    m_alert_dispatcher->On<lt::add_torrent_alert>([this](auto* a)          { OnAddTorrentAlert(a); });
    m_alert_dispatcher->On<lt::alerts_dropped_alert>([this](auto* a)       { OnAlertsDroppedAlert(a); });
    m_alert_dispatcher->On<lt::listen_failed_alert>([this](auto* a)        { OnListenFailedAlert(a); });
    m_alert_dispatcher->On<lt::listen_succeeded_alert>([this](auto* a)     { OnListenSucceededAlert(a); });
    m_alert_dispatcher->On<lt::metadata_received_alert>([this](auto* a)    { OnMetadataReceivedAlert(a); });
    m_alert_dispatcher->On<lt::save_resume_data_alert>([this](auto* a)     { OnSaveResumeDataAlert(a); });
    m_alert_dispatcher->On<lt::session_stats_alert>([this](auto* a)        { OnSessionStatsAlert(a); });
    m_alert_dispatcher->On<lt::state_update_alert>([this](auto* a)         { OnStateUpdateAlert(a); });
    m_alert_dispatcher->On<lt::storage_moved_alert>([this](auto* a)        { OnStorageMovedAlert(a); });
    m_alert_dispatcher->On<lt::storage_moved_failed_alert>([this](auto* a) { OnStorageMovedFailedAlert(a); });
    m_alert_dispatcher->On<lt::torrent_checked_alert>([this](auto* a)      { OnTorrentCheckedAlert(a); });
    m_alert_dispatcher->On<lt::torrent_error_alert>([this](auto* a)        { OnTorrentErrorAlert(a); });
    m_alert_dispatcher->On<lt::torrent_finished_alert>([this](auto* a)     { OnTorrentFinishedAlert(a); });
    m_alert_dispatcher->On<lt::torrent_paused_alert>([this](auto* a)       { OnTorrentPausedAlert(a); });
    m_alert_dispatcher->On<lt::torrent_removed_alert>([this](auto* a)      { OnTorrentRemovedAlert(a); });
    m_alert_dispatcher->On<lt::torrent_resumed_alert>([this](auto* a)      { OnTorrentResumedAlert(a); });
    m_alert_dispatcher->On<lt::tracker_error_alert>([this](auto* a)        { OnTrackerErrorAlert(a); });

    Forward<lt::file_error_alert>("torrent.file_error");
    Forward<lt::state_changed_alert>("torrent.state_changed");
    Forward<lt::tracker_reply_alert>("torrent.tracker_reply");
    Forward<lt::tracker_warning_alert>("torrent.tracker_warning");
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

    // process all libtorrent alerts to make sure missings adds
    // really are missing
    ReadAlerts();

    std::unordered_set<lt::info_hash_t>                                live;
    std::optional<std::map<const TorrentClientData*, lt::info_hash_t>> owners;

    int tracked   = 0;
    int loaded    = 0;
    int rehashed  = 0;
    int untracked = 0;
    int failed    = 0;

    const Transaction tx(m_options.db);

    for (const auto& th : handles)
    {
        const auto hash = th.info_hashes();

        live.insert(hash);

        auto it = m_torrents.find(hash);

        if (it == m_torrents.end())
        {
            if (!owners)
            {
                owners.emplace();

                for (const auto& [ h, t ] : m_torrents)
                {
                    if (t.data != nullptr)
                    {
                        owners->emplace(t.data.get(), h);
                    }
                }
            }

            if (const auto owner = owners->find(th.userdata().get<TorrentClientData>()); owner != owners->end())
            {
                UpdateInfoHashes(owner->second, hash);

                it = m_torrents.find(hash);

                if (it->second.state == Torrent::State::Current)
                {
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

                        it->second.metadata_saved = params.ti != nullptr;
                    }
                    catch(const std::exception& e)
                    {
                        BOOST_LOG_TRIVIAL(error) << Log(hash) << "Failed to store torrent with updated hashes: " << e.what();
                    }

                    rehashed++;

                    continue;
                }
            }
        }

        if (it != m_torrents.end())
        {
            if (it->second.state == Torrent::State::Current)
            {
                continue;
            }

            const bool from_db = it->second.state == Torrent::State::Loading;

            it->second.state  = Torrent::State::Current;
            it->second.status = th.status();

            if (from_db)
            {
                loaded++;
                continue;
            }
        }
        else
        {
            // a torrent in libtorrent that Porla isn't trackign
            Track(th.status());
            it = m_torrents.find(hash);
        }

        const auto* data = it->second.data.get();

        try
        {
            const auto params = th.get_resume_data(lt::torrent_handle::save_info_dict);

            AddTorrentParams::Insert(
                m_options.db,
                m_id,
                hash,
                params,
                data == nullptr ? TorrentClientData{} : *data,
                static_cast<int>(it->second.status.queue_position));
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
            const auto it = m_torrents.find(hash);

            if (it->second.state == Torrent::State::Current)
            {
                UntrackTorrent(hash);
                untracked++;
                continue;
            }

            m_torrents.erase(it);

            failed++;
        }
    }

    if (tracked + loaded + rehashed + untracked + failed == 0)
    {
        return;
    }

    BOOST_LOG_TRIVIAL(warning)
        << Log() << "Reconciled torrents:"
        << " tracked=" << tracked
        << " loaded=" << loaded
        << " rehashed=" << rehashed
        << " untracked=" << untracked
        << " failed=" << failed;
}

bool Session::Track(const lt::torrent_status& status)
{
    const auto [ it, inserted ] = m_torrents.try_emplace(status.info_hashes);

    it->second.state  = Torrent::State::Current;
    it->second.status = status;

    if (inserted)
    {
        it->second.data.reset(status.handle.userdata().get<TorrentClientData>());

        // update adopted torrents
        it->second.metadata_announced = status.has_metadata;
        it->second.metadata_saved     = status.has_metadata;
    }

    return inserted;
}

void Session::UntrackInvalidTorrents()
{
    std::vector<lt::info_hash_t> gone;

    for (const auto& [ hash, t ] : m_torrents)
    {
        // torrents that are still begin added have no handle yet
        if (t.state == Torrent::State::Current && !t.status.handle.is_valid())
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
    const auto it = m_torrents.find(hash);

    const bool is_current = it != m_torrents.end()
        && it->second.state == Torrent::State::Current;

    if (it != m_torrents.end())
    {
        m_torrents.erase(it);
    }

    if (is_current)
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
