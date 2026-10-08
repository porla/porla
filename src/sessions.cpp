#include "sessions.hpp"

#include <filesystem>
#include <fstream>

#include <boost/log/trivial.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/extensions/ut_metadata.hpp>
#include <libtorrent/extensions/ut_pex.hpp>
#include <libtorrent/extensions/smart_ban.hpp>
#include <libtorrent/session_stats.hpp>

#include "data/models/addtorrentparams.hpp"
#include "data/models/sessions.hpp"
#include "data/transaction.hpp"
#include "events.hpp"
#include "json/all.hpp"
#include "timer.hpp"
#include "torrentclientdata.hpp"
#include "utils/hex.hpp"

namespace fs = std::filesystem;

using porla::Data::Models::AddTorrentParams;
using porla::Data::Transaction;
using porla::Sessions;
using porla::SessionsOptions;

namespace
{
    const std::vector<lt::stats_metric> kSessionMetrics = lt::session_stats_metrics();

    static constexpr int kLoadChunkSize      = 100;
    static constexpr int  kLoadMaxChunkErrors = 8;
    static constexpr auto kLoadRetryBaseDelay = std::chrono::milliseconds(100);
    static constexpr auto kLoadRetryMaxDelay  = std::chrono::milliseconds(5000);

    std::string Sub(const lt::info_hash_t& ih)
    {
        return ih.has_v1()
            ? porla::Utils::ToHex({ ih.v1.data(), static_cast<std::size_t>(ih.v1.size()) })
            : porla::Utils::ToHex({ ih.v2.data(), static_cast<std::size_t>(ih.v2.size()) });
    }

    std::string Sub(const Sessions::SessionStatePtr& state, const lt::info_hash_t& ih)
    {
        const auto& h = Sub(ih);

        return "session[" + state->name + "][" + h.substr(0, 8) + "] ";
    }

    std::string Sub(const Sessions::SessionStatePtr& state)
    {
        return "session[" + state->name + "] ";
    }
}

struct Sessions::SessionState::LoadState
{
    AddTorrentParams::Cursor cursor;
    int                      count;
    int                      loaded;
    int                      chunks;
    int                      errors;
    bool                     failed;
    std::function<void()>    callback;
};

Sessions::SessionState::~SessionState()
{
    if (session == nullptr)
    {
        return;
    }

    std::vector<TorrentClientData*> data;

    try
    {
        for (const auto& th : session->get_torrents())
        {
            if (auto* client_data = th.userdata().get<TorrentClientData>())
            {
                data.push_back(client_data);
            }
        }
    }
    catch(const std::exception& e)
    {
        BOOST_LOG_TRIVIAL(error) << "session[" << name << "] Failed to list torrents on teardown: " << e.what();
    }

    session.reset();

    for (auto* d : data)
    {
        delete d;
    }
}

void Sessions::SessionState::Recheck(const lt::info_hash_t& hash)
{
    const auto it = torrents.find(hash);

    if (it == torrents.end())
    {
        BOOST_LOG_TRIVIAL(warning)
            << "session[" << name << "][" << Sub(hash).substr(0, 8) << "] Failed to find torrent for rechecking";
        return;
    }

    const auto& handle = it->second.handle;

    // If the torrent is paused, it must be resumed in order to be rechecked.
    // It should also not be auto managed, so remove it from that as well.
    // When the session posts a torrent_check alert, restore its flags.

    const auto alert_type     = lt::torrent_checked_alert::alert_type;
    const auto original_flags = it->second.handle.flags();

    if ((original_flags & lt::torrent_flags::auto_managed) == lt::torrent_flags::auto_managed)
    {
        handle.unset_flags(lt::torrent_flags::auto_managed);
    }

    if ((original_flags & lt::torrent_flags::paused) == lt::torrent_flags::paused)
    {
        handle.resume();
    }

    m_oneshot_torrent_callbacks[{ alert_type, hash }].emplace_back(
        [hash, original_flags](const std::shared_ptr<SessionState>& state)
        {
            const auto it = state->torrents.find(hash);

            if (it == state->torrents.end())
            {
                return;
            }

            const auto& th = it->second.handle;

            if (!th.is_valid())
            {
                return;
            }

            // TODO: Unsure about the order here. If there are reports that force-checking a torrent
            //       leads to any issues with resume/pause, the order of these statements might matter.

            if ((original_flags & lt::torrent_flags::auto_managed) == lt::torrent_flags::auto_managed)
            {
                th.set_flags(lt::torrent_flags::auto_managed);
            }

            if ((original_flags & lt::torrent_flags::paused) == lt::torrent_flags::paused)
            {
                th.pause();
            }
        });

    handle.force_recheck();
}

Sessions::Sessions(const SessionsOptions &options)
    : m_options(options)
{
}

Sessions::~Sessions()
{
    std::vector<SessionStatePtr> pending;
    pending.reserve(m_sessions.size());

    BOOST_LOG_TRIVIAL(info) << "Shutting down " << pending.size() << " session(s)";

    while (!m_sessions.empty())
    {
        auto node  = m_sessions.extract(m_sessions.begin());
        auto state = std::move(node.mapped());

        try
        {
            UnloadSession(state);
        }
        catch(const std::exception& e)
        {
            BOOST_LOG_TRIVIAL(error) << "session[" << state->name << "] Failed to unload session: " << e.what();
        }

        pending.push_back(std::move(state));
    }

    BOOST_LOG_TRIVIAL(info) << "All state saved";

    for (auto& state : pending)
    {
        const auto name = state->name;

        BOOST_LOG_TRIVIAL(info)
            << "session[" << name << "] Destroying session - this is safe to interrupt";

        std::weak_ptr<SessionState> observer = state;

        state.reset();

        if (!observer.expired())
        {
            BOOST_LOG_TRIVIAL(warning)
                << "session[" << name << "] Still referenced after unload - "
                << "destruction deferred past shutdown";
        }
    }
}

std::shared_ptr<Sessions::SessionState> Sessions::Get(const int id)
{
    const auto it = m_sessions.find(id);

    return it == m_sessions.end()
        ? nullptr
        : it->second;
}

int Sessions::Add(const Data::Models::Sessions::Session& session)
{
    const int id = Data::Models::Sessions::Insert(m_options.db, session);

    BOOST_LOG_TRIVIAL(info) << "session[" << session.name << "] Added - loading";

    LoadById(id);

    if (const auto state = Get(id))
    {
        Publish(state, SessionEvent("session.added"));
    }

    return id;
}

void Sessions::Remove(int id)
{
    const auto session = Data::Models::Sessions::GetById(m_options.db, id);

    if (!session)
    {
        return;
    }

    UnloadById(id);

    Data::Models::Sessions::Remove(m_options.db, id);

    BOOST_LOG_TRIVIAL(info) << "session[" << session->name << "] Removed";

    PublishDetached(id, SessionEvent("session.removed", {{ "session_name", session->name }}));
}

void Sessions::Update(const Data::Models::Sessions::Session& session)
{
    Data::Models::Sessions::Update(m_options.db, session);

    if (const auto state = Get(session.id))
    {
        state->name = session.name;

        Publish(state, SessionEvent("session.updated"));
    }
}

void Sessions::Load(const std::function<void()>& callback)
{
    const auto sessions = Data::Models::Sessions::List(m_options.db);

    BOOST_LOG_TRIVIAL(info) << "Loading " << sessions.size() << " session(s)";

    if (sessions.empty())
    {
        if (callback)
        {
            boost::asio::post(m_options.io, callback);
        }

        return;
    }

    // All sessions are loaded concurrently. After each one has loaded, this
    // counter is decreased by one. When it hits zero, the callback is invoked.
    auto outstanding_sessions = std::make_shared<std::size_t>(sessions.size());

    auto single_session_loaded = [this, outstanding_sessions, callback]()
    {
        --(*outstanding_sessions);

        if (*outstanding_sessions > 0)
        {
            return;
        }

        if (callback)
        {
            boost::asio::post(m_options.io, callback);
        }
    };

    for (const auto& session : sessions)
    {
        LoadById(session.id, single_session_loaded);
    }
}

void Sessions::LoadById(int id, const std::function<void()>& callback)
{
    const auto s = Data::Models::Sessions::GetById(m_options.db, id);

    if (!s)
    {
        BOOST_LOG_TRIVIAL(warning) << "No session with ID " << id;
        if (callback) { callback(); }
        return;
    }

    const auto session = s.value();

    if (m_sessions.contains(session.id))
    {
        BOOST_LOG_TRIVIAL(warning) << "session[" << session.name << "] Already loaded - skipping";
        if (callback) { callback(); }
        return;
    }

    auto state = std::make_shared<SessionState>();
    state->id = session.id;
    state->name = session.name;
    state->session = std::make_unique<lt::session>(std::move(session.params));
    state->session->add_extension(&lt::create_ut_metadata_plugin);
    state->session->add_extension(&lt::create_ut_pex_plugin);
    state->session->add_extension(&lt::create_smart_ban_plugin);
    state->torrents = {};

    state->session->set_alert_notify(
        [this, weak = std::weak_ptr(state)]()
        {
            boost::asio::post(
                m_options.io,
                [this, weak] { if (auto state = weak.lock()) { ReadAlerts(state); } });
        });

    state->m_timers.emplace_back(Timer::Create(m_options.io, session.timer_dht_stats, [this, w = std::weak_ptr(state)] { if (auto state = w.lock()) { PostDhtStats(state); } }));
    state->m_timers.emplace_back(Timer::Create(m_options.io, session.timer_save_state, [this, w = std::weak_ptr(state)] { if (auto state = w.lock()) { SaveState(state); } }));
    state->m_timers.emplace_back(Timer::Create(m_options.io, session.timer_session_stats, [this, w = std::weak_ptr(state)] { if (auto state = w.lock()) { PostSessionStats(state); } }));
    state->m_timers.emplace_back(Timer::Create(m_options.io, session.timer_torrent_updates, [this, w = std::weak_ptr(state)] { if (auto state = w.lock()) { PostTorrentUpdates(state); } }));

    state->m_load_state = std::make_unique<SessionState::LoadState>(
        SessionState::LoadState{
            .cursor   = {},
            .count    = AddTorrentParams::Count(m_options.db, session.id),
            .loaded   = 0,
            .chunks   = 0,
            .errors   = 0,
            .failed   = false,
            .callback = callback
        });

    BOOST_LOG_TRIVIAL(info)
        << Sub(state) << "Loading " << state->m_load_state->count
        << " torrent(s) from storage";

    m_sessions.insert({ state->id, state });

    boost::asio::post(m_options.io, [this, weak = std::weak_ptr(state)]()
    {
        if (const auto state = weak.lock()) { LoadTorrentsChunk(state); }
    });
}

void Sessions::LoadTorrentsChunk(const SessionStatePtr& state)
{
    if (state->m_load_state == nullptr)
    {
        return;
    }

    try
    {   
        ReadAlerts(state);
    }
    catch(const std::exception& e)
    {
        BOOST_LOG_TRIVIAL(error) << Sub(state) << "Failed to read alerts during load: " << e.what();
    }

    if (state->m_load_state == nullptr)
    {
        return;
    }

    auto& load = *state->m_load_state;
    bool  more = false;

    try
    {
        more = AddTorrentParams::Next(
            m_options.db,
            state->id,
            load.cursor,
            kLoadChunkSize,
            [&state, &load](lt::add_torrent_params& params)
            {
                if (state->m_adding.contains(params.info_hashes)
                    || state->torrents.contains(params.info_hashes))
                {
                    return;
                }

                params.userdata.get<TorrentClientData>()->state = state;

                state->m_adding.insert(params.info_hashes);
                state->session->async_add_torrent(params);

                load.loaded++;
            });

        load.errors = 0;
    }
    catch(const std::exception& e)
    {
        load.errors++;

        if (load.errors <= kLoadMaxChunkErrors)
        {
            const auto delay = std::min(
                kLoadRetryMaxDelay,
                kLoadRetryBaseDelay * (1 << (load.errors - 1)));

            BOOST_LOG_TRIVIAL(warning)
                << Sub(state) << "Chunk failed at " << load.loaded
                << " of " << load.count << " (attempt " << load.errors << " of "
                << kLoadMaxChunkErrors << ", retrying in " << delay.count()
                << "ms): " << e.what();

            auto retry = std::make_shared<boost::asio::steady_timer>(m_options.io, delay);

            retry->async_wait(
                [this, retry, weak = std::weak_ptr(state)](const boost::system::error_code& ec)
                {
                    if (ec) { return; }
                    if (const auto state = weak.lock()) { LoadTorrentsChunk(state); }
                });
            return;
        }

        BOOST_LOG_TRIVIAL(error)
            << Sub(state) << "Failed to load torrents after "
            << load.loaded << " of " << load.count << ": " << e.what();

        load.failed = true;
        more        = false;
    }

    if (!more)
    {
        if (load.loaded > 0)
        {
            try
            {
                const auto& all_statuses = state->session->get_torrent_status(
                    [](const auto& ts) { return true; },
                    lt::status_flags_t::all());

                for (const auto& ts : all_statuses)
                {
                    state->torrents.insert_or_assign(ts.info_hashes, ts);
                }

                // every loaded torrent is tracked now - add alerts still on their way, or lost,
                // no longer need telling apart from new torrents
                state->m_adding.clear();
            }
            catch(const std::exception& e)
            {
                BOOST_LOG_TRIVIAL(error)
                    << Sub(state) << "Failed to read torrent status after load: " << e.what();
            }
        }

        if (load.count > 0)
        {
            if (load.failed)
            {
                const auto remaining = load.count - load.loaded;

                BOOST_LOG_TRIVIAL(error)
                    << Sub(state) << "Incomplete load - "
                    << load.loaded << " of " << load.count
                    << " torrent(s) were added. The remaining "
                    << remaining << " are still in the database but not in the session. "
                    << "Do not re-add them; restart Porla once the database is healthy.";
            }
            else
            {
                BOOST_LOG_TRIVIAL(info)
                    << Sub(state) << "Added " << load.loaded
                    << " (of " << load.count << ") torrent(s) to the session";
            }
        }

        Publish(state, SessionEvent("session.loaded", {
            { "torrents", load.loaded },
            { "failed",   load.failed }
        }));

        FinishLoad(state);

        // a reconcile was requested during loading. schedule it now
        if (state->m_reconcile_pending)
        {
            state->m_reconcile_pending = false;
            ScheduleReconcileTorrents(state);
        }

        return;
    }

    load.chunks++;

    if (load.chunks % 10 == 0)
    {
        BOOST_LOG_TRIVIAL(info)
            << Sub(state) << load.loaded << " torrents (of "
            << load.count << ") added";
    }

    boost::asio::post(m_options.io, [this, weak = std::weak_ptr(state)]()
    {
        if (const auto state = weak.lock()) { LoadTorrentsChunk(state); }
    });
}

void Sessions::FinishLoad(const SessionStatePtr& state)
{
    if (state->m_load_state == nullptr)
    {
        return;
    }

    auto callback = std::move(state->m_load_state->callback);

    state->m_load_state.reset();

    if (!callback)
    {
        return;
    }

    try
    {
        callback();
    }
    catch(const std::exception& e)
    {
        BOOST_LOG_TRIVIAL(error)
            << Sub(state) << "Load completion callback failed: " << e.what();
    }
}

void Sessions::SaveSessionParams(const SessionStatePtr& state)
{
    auto session = Data::Models::Sessions::GetById(m_options.db, state->id);

    if (!session)
    {
        return;
    }

    session->params = state->session->session_state();

    Data::Models::Sessions::Update(m_options.db, *session);
}

void Sessions::UnloadById(int id)
{
    const auto it = m_sessions.find(id);

    if (it == m_sessions.end())
    {
        return;
    }

    const auto name = it->second->name;

    UnloadSession(it->second);

    m_sessions.erase(it);

    PublishDetached(id, SessionEvent("session.unloaded", {{ "session_name", name }}));
}

void Sessions::ReadAlerts(const std::shared_ptr<SessionState>& state)
{
    std::vector<lt::alert*> alerts;
    state->session->pop_alerts(&alerts);

    if (alerts.empty())
    {
        return;
    }

    const Transaction tx(m_options.db);

    for (const auto alert : alerts)
    {
        try
        {
            ProcessAlert(state, alert);
        }
        catch(const std::exception& e)
        {
            BOOST_LOG_TRIVIAL(error)
                << Sub(state) << "Failed to process alert " << alert->what() << ": " << e.what();
        }
    }
}

void Sessions::ProcessAlert(const SessionStatePtr& state, const lt::alert* alert)
{
    BOOST_LOG_TRIVIAL(trace) << Sub(state) << alert->what() << ": " << alert->message();

    switch (alert->type())
    {
        case lt::add_torrent_alert::alert_type:      OnAddTorrentAlert(state, lt::alert_cast<lt::add_torrent_alert>(alert));          break;
        case lt::file_error_alert::alert_type:       OnFileErrorAlert(state, lt::alert_cast<lt::file_error_alert>(alert));            break;
        case lt::save_resume_data_alert::alert_type: OnSaveResumeDataAlert(state, lt::alert_cast<lt::save_resume_data_alert>(alert)); break;
        case lt::torrent_removed_alert::alert_type:  OnTorrentRemovedAlert(state, lt::alert_cast<lt::torrent_removed_alert>(alert));  break;
        case lt::torrent_resumed_alert::alert_type:  OnTorrentResumedAlert(state, lt::alert_cast<lt::torrent_resumed_alert>(alert));  break;

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
                << Sub(state) << "The libtorrent alert queue is full and dropped "
                << types << " alerts. Consider raising alert_queue_size.";

            if (ada->dropped_alerts.test(lt::add_torrent_alert::alert_type)
                || ada->dropped_alerts.test(lt::torrent_removed_alert::alert_type))
            {
                ScheduleReconcileTorrents(state);
            }

            break;
        }

        case lt::listen_failed_alert::alert_type:
        {
            const auto lfa = lt::alert_cast<lt::listen_failed_alert>(alert);
            BOOST_LOG_TRIVIAL(warning) << Sub(state) << lfa->message();
            break;
        }
        case lt::listen_succeeded_alert::alert_type:
        {
            const auto lsa = lt::alert_cast<lt::listen_succeeded_alert>(alert);
            BOOST_LOG_TRIVIAL(info) << Sub(state) << lsa->message();
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

            BOOST_LOG_TRIVIAL(info) << Sub(state, info_hashes) << "Metadata received";

            // A magnet link only carries the hash(es) it was added with. A hybrid
            // torrent gains its other hash with the metadata, and every later alert
            // uses the full hashes, so move the torrent to its new key.
            if (!state->torrents.contains(info_hashes))
            {
                const auto it = std::find_if(
                    state->torrents.begin(),
                    state->torrents.end(),
                    [&](const auto& kv) { return kv.second.handle == mra->handle; });

                if (it != state->torrents.end())
                {
                    UpdateInfoHashes(state, it->first, info_hashes);
                }
            }

            mra->handle.save_resume_data(
                lt::torrent_handle::save_info_dict);

            EmitTorrentEvent(state, "torrent.metadata_received", *mra);

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

            Publish(state, SessionEvent("session.stats", {{ "stats", std::move(stats) }}));

            break;
        }
        case lt::state_update_alert::alert_type:
        {
            auto sua = lt::alert_cast<lt::state_update_alert>(alert);

            for (const auto& status : sua->status)
            {
                const auto it = state->torrents.find(status.info_hashes);

                if (it == state->torrents.end())
                {
                    BOOST_LOG_TRIVIAL(debug)
                        << Sub(state, status.info_hashes) << "Received state update for non-tracked torrent";

                    ScheduleReconcileTorrents(state);

                    continue;
                }

                it->second = status;
            }

            boost::asio::post(m_options.io, [this, weak = std::weak_ptr(state), status = sua->status]()
            {
                if (auto state = weak.lock()) { m_state_update(state, status); }
            });

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
                << Sub(state, sma->handle.info_hashes()) << "Storage moved to " << sma->storage_path();

            sma->handle.save_resume_data(
                lt::torrent_handle::only_if_modified);

            sma->handle.post_status();

            EmitTorrentEvent(state, "torrent.moved", *sma);

            break;
        }
        case lt::storage_moved_failed_alert::alert_type:
        {
            const auto smfa = lt::alert_cast<lt::storage_moved_failed_alert>(alert);

            if (!smfa->handle.is_valid())
            {
                break;
            }

            BOOST_LOG_TRIVIAL(warning) << Sub(state, smfa->handle.info_hashes()) << smfa->message();

            EmitTorrentEvent(state, "torrent.move_failed", *smfa);

            break;
        }
        case lt::state_changed_alert::alert_type:
        {
            const auto sca = lt::alert_cast<lt::state_changed_alert>(alert);

            EmitTorrentEvent(state, "torrent.state_changed", *sca);

            break;
        }
        case lt::torrent_error_alert::alert_type:
        {
            const auto tea = lt::alert_cast<lt::torrent_error_alert>(alert);

            if (!tea->handle.is_valid())
            {
                break;
            }

            BOOST_LOG_TRIVIAL(error) << Sub(state, tea->handle.info_hashes()) << tea->message();

            EmitTorrentEvent(state, "torrent.error", *tea);

            break;
        }
        case lt::torrent_checked_alert::alert_type:
        {
            const auto tca = lt::alert_cast<lt::torrent_checked_alert>(alert);

            if (!tca->handle.is_valid())
            {
                break;
            }

            BOOST_LOG_TRIVIAL(info) << Sub(state, tca->handle.info_hashes())
                << "Torrent finished checking";

            if (auto node = state->m_oneshot_torrent_callbacks.extract({ alert->type(), tca->handle.info_hashes() }))
            {
                for (auto&& cb : node.mapped()) { cb(state); }
            }

            EmitTorrentEvent(state, "torrent.checked", *tca);

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
                    state->id,
                    status.info_hashes,
                    *client_data);

                first = status.total_payload_download > 0;
            }

            if (first)
            {
                BOOST_LOG_TRIVIAL(info) << Sub(state, status.info_hashes) << "Torrent finished";
            }

            status.handle.save_resume_data(
                lt::torrent_handle::only_if_modified);

            EmitTorrentEvent(state, "torrent.finished", *tfa, { { "first", first } });

            break;
        }
        case lt::torrent_paused_alert::alert_type:
        {
            const auto tpa = lt::alert_cast<lt::torrent_paused_alert>(alert);

            BOOST_LOG_TRIVIAL(debug)
                << Sub(state, tpa->handle.info_hashes()) << "Torrent paused";

            EmitTorrentEvent(state, "torrent.paused", *tpa);

            break;
        }
        case lt::tracker_error_alert::alert_type:
        {
            const auto tea = lt::alert_cast<lt::tracker_error_alert>(alert);

            if (tea->error == lt::errors::announce_skipped)
            {
                break;
            }

            EmitTorrentEvent(state, "tracker.error", *tea);

            break;
        }
        case lt::tracker_reply_alert::alert_type:
        {
            const auto tra = lt::alert_cast<lt::tracker_reply_alert>(alert);

            EmitTorrentEvent(state, "tracker.reply", *tra);

            break;
        }
        case lt::tracker_warning_alert::alert_type:
        {
            const auto twa = lt::alert_cast<lt::tracker_warning_alert>(alert);

            EmitTorrentEvent(state, "tracker.warning", *twa);

            break;
        }
    }
}

void Sessions::PostDhtStats(const std::shared_ptr<SessionState>& state)
{
    state->session->post_dht_stats();
}

void Sessions::PostSessionStats(const std::shared_ptr<SessionState>& state)
{
    state->session->post_session_stats();
}

void Sessions::PostTorrentUpdates(const std::shared_ptr<SessionState>& state)
{
    state->session->post_torrent_updates();
}

void Sessions::SaveState(const std::shared_ptr<SessionState>& state)
{
    SaveSessionParams(state);

    UntrackInvalidTorrents(state);

    std::vector<lt::torrent_status> torrents = state->session->get_torrent_status(
        [](lt::torrent_status const& ts)
        {
            return bool(ts.need_save_resume_data & lt::torrent_handle::only_if_modified);
        });

    if (torrents.empty())
    {
        return;
    }

    BOOST_LOG_TRIVIAL(info) << Sub(state) << "Saving state for " << torrents.size() << " torrent(s) in session " << state->name;

    for (const auto& ts : torrents)
    {
        ts.handle.save_resume_data(
            lt::torrent_handle::only_if_modified);
    }
}

void Sessions::UnloadSession(const std::shared_ptr<SessionState>& state)
{
    FinishLoad(state);

    state->m_timers.clear();
    state->session->set_alert_notify({});

    SaveSessionParams(state);

    state->session->pause();

    auto torrents = state->session->get_torrent_status(
        [](const lt::torrent_status& ts)
        {
            return ts.has_metadata && bool(ts.need_save_resume_data);
        });

    int chunk_size = 1000;
    int chunks     = static_cast<int>(torrents.size() / chunk_size) + 1;

    BOOST_LOG_TRIVIAL(info) << Sub(state)
                            << "Saving resume data in " << chunks << " chunk(s) - total torrents: "
                            << torrents.size();

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
                    << Sub(state) << "Failed to post save resume data: " << e.what();
            }

            std::advance(current, 1);
        }

        BOOST_LOG_TRIVIAL(info) << Sub(state)
                                << "Chunk " << i + 1 << " - Saving state for " << outstanding
                                << " torrent(s) (out of " << chunk_items << ")";

        while (outstanding > 0) {
            const auto found_alert = state->session->wait_for_alert(lt::seconds(10));

            if (!found_alert)
            {
                BOOST_LOG_TRIVIAL(warning)
                    << Sub(state) << "Gave up waiting for " << outstanding << " resume data alerts "
                    << "in chunk " << i + 1;

                break;
            }

            std::vector<lt::alert *> alerts;
            state->session->pop_alerts(&alerts);

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
                        << Sub(state, fail->handle.info_hashes())
                        << "Failed to save resume data: " << fail->message();

                    continue;
                }

                if (auto removed = lt::alert_cast<lt::torrent_removed_alert>(a))
                {
                    OnTorrentRemovedAlert(state, removed);
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
                        state->id,
                        info_hash,
                        rd->params,
                        data,
                        static_cast<int>(rd->handle.queue_position()));

                    if (rd->params.ti)
                    {
                        AddTorrentParams::InsertTorrentInfo(
                            m_options.db,
                            state->id,
                            info_hash,
                            *rd->params.ti);
                    }
                }
                catch(const std::exception& e)
                {
                    BOOST_LOG_TRIVIAL(error) << Sub(state, rd->params.info_hashes)
                        << "Failed to save resume data: " << e.what();
                }
            }
        }
    }
}

void Sessions::OnAddTorrentAlert(const SessionStatePtr& state, const lt::add_torrent_alert* alert)
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
            << Sub(state, info_hash)
            << "Failed to add torrent: " << alert->error.what();

        state->m_adding.erase(info_hash);

        delete alert->params.userdata.get<TorrentClientData>();

        return;
    }

    if (state->m_adding.erase(alert->handle.info_hashes()) > 0)
    {
        lt::torrent_status status;
        status.handle      = alert->handle;
        status.info_hashes = alert->handle.info_hashes();

        state->torrents.try_emplace(alert->handle.info_hashes(), status);

        return;
    }

    const auto data   = alert->handle.userdata().get<TorrentClientData>();
    const auto status = alert->handle.status();

    const auto [ _, inserted ] = state->torrents.insert_or_assign(
        alert->handle.info_hashes(),
        status);

    if (!inserted)
    {
        BOOST_LOG_TRIVIAL(debug)
            << Sub(state, status.info_hashes)
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
            << Sub(state, status.info_hashes) << "Missing userdata for torrent";
    }

    const TorrentClientData fallback;

    try
    {
        AddTorrentParams::Insert(
            m_options.db,
            state->id,
            alert->handle.info_hashes(),
            alert->params,
            data == nullptr ? fallback : *data,
            static_cast<int>(status.queue_position));

        if (alert->params.ti)
        {
            AddTorrentParams::InsertTorrentInfo(
                m_options.db,
                state->id,
                alert->handle.info_hashes(),
                *alert->params.ti);
        }
    }
    catch(const std::exception& e)
    {
        BOOST_LOG_TRIVIAL(error)
            << Sub(state, status.info_hashes) << "Failed to insert: " << e.what();
    }

    alert->handle.save_resume_data(
        lt::torrent_handle::only_if_modified);

    EmitTorrentEvent(state, "torrent.added", *alert);
}

void Sessions::OnFileErrorAlert(const SessionStatePtr& state, const lt::file_error_alert* alert)
{
    EmitTorrentEvent(state, "torrent.file_error", *alert);
}

void Sessions::OnSaveResumeDataAlert(const SessionStatePtr& state, const lt::save_resume_data_alert* alert)
{
    if (!alert->handle.is_valid())
    {
        BOOST_LOG_TRIVIAL(debug)
            << Sub(state, alert->params.info_hashes) << "Received resume data for invalid torrent";

        return;
    }

    const auto data        = alert->handle.userdata().get<TorrentClientData>();
    const auto info_hashes = alert->handle.info_hashes();

    AddTorrentParams::Update(
        m_options.db,
        state->id,
        info_hashes,
        alert->params,
        data,
        static_cast<int>(alert->handle.queue_position()));

    if (alert->params.ti)
    {
        AddTorrentParams::InsertTorrentInfo(
            m_options.db,
            state->id,
            info_hashes,
            *alert->params.ti);
    }

    BOOST_LOG_TRIVIAL(debug) << Sub(state, info_hashes) << "Resume data saved";
}

void Sessions::OnTorrentRemovedAlert(const SessionStatePtr& state, const lt::torrent_removed_alert* alert)
{
    BOOST_LOG_TRIVIAL(info)
        << Sub(state, alert->info_hashes) << "Torrent removed";

    UntrackTorrent(state, alert->info_hashes);

    delete alert->userdata.get<TorrentClientData>();
}

void Sessions::OnTorrentResumedAlert(const SessionStatePtr& state, const lt::torrent_resumed_alert* alert)
{
    BOOST_LOG_TRIVIAL(debug)
        << Sub(state, alert->handle.info_hashes())
        << "Torrent resumed";

    EmitTorrentEvent(state, "torrent.resumed", *alert);
}

template <typename T>
void Sessions::Publish(const SessionStatePtr& state, T event)
{
    static_assert(std::is_base_of_v<SessionEvent, T>);

    event.session_id = state->id;
    event.session    = state;

    m_options.events.Publish(std::move(event));
}

template <typename T>
void Sessions::PublishDetached(int session_id, T event)
{
    static_assert(std::is_base_of_v<SessionEvent, T>);

    event.session_id = session_id;

    m_options.events.Publish(std::move(event));
}

template<typename Alert>
void Sessions::EmitTorrentEvent(const SessionStatePtr& state, std::string name, const Alert& alert, nlohmann::json extra)
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

    Publish(state, std::move(event));
}

void Sessions::UntrackInvalidTorrents(const SessionStatePtr& state)
{
    std::vector<lt::info_hash_t> gone;

    for (const auto& [ hash, status ] : state->torrents)
    {
        if (!status.handle.is_valid())
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
        UntrackTorrent(state, hash);
    }

    BOOST_LOG_TRIVIAL(warning)
        << Sub(state) << "Removed " << gone.size() << " orphaned torrent(s). "
        << "This might be due to dropped alerts. Consider increasing alert_queue_size";
}

void Sessions::UntrackTorrent(const SessionStatePtr& state, const lt::info_hash_t& hash)
{
    state->m_adding.erase(hash);

    const bool tracked = state->torrents.erase(hash) > 0;

    std::erase_if(
        state->m_oneshot_torrent_callbacks,
        [&](const auto& kv)
        {
            return kv.first.second == hash;
        });

    if (tracked)
    {
        boost::asio::post(
            m_options.io,
            [this, weak = std::weak_ptr(state), hash]()
            {
                if (auto state = weak.lock()) { m_torrent_removed(state, hash); }
            });

        TorrentEvent removed("torrent.removed");
        removed.info_hash = hash;

        Publish(state, std::move(removed));
    }

    try
    {
        AddTorrentParams::Remove(m_options.db, state->id, hash);
    }
    catch(const std::exception& e)
    {
        BOOST_LOG_TRIVIAL(error)
            << Sub(state, hash) << "Failed to remove torrent from database: "
            << e.what();
    }
}

void Sessions::UpdateInfoHashes(const SessionStatePtr& state, const lt::info_hash_t& prev, const lt::info_hash_t& curr)
{
    auto node = state->torrents.extract(prev);

    if (node.empty())
    {
        return;
    }

    node.key() = curr;
    node.mapped().info_hashes = curr;

    state->torrents.insert(std::move(node));

    for (auto cb = state->m_oneshot_torrent_callbacks.begin(); cb != state->m_oneshot_torrent_callbacks.end();)
    {
        if (cb->first.second != prev)
        {
            ++cb;
            continue;
        }

        auto& callbacks = state->m_oneshot_torrent_callbacks[{ cb->first.first, curr }];
        std::move(cb->second.begin(), cb->second.end(), std::back_inserter(callbacks));

        cb = state->m_oneshot_torrent_callbacks.erase(cb);
    }
}

void Sessions::ScheduleReconcileTorrents(const SessionStatePtr& state)
{
    if (state->m_reconcile_pending)
    {
        return;
    }

    state->m_reconcile_pending = true;

    // if the reconcile is requested during loading, hold it off
    // until all chunks have been loaded
    if (state->m_load_state != nullptr)
    {
        return;
    }

    boost::asio::post(
        m_options.io,
        [this, weak = std::weak_ptr(state)]()
        {
            if (auto state = weak.lock())
            {
                ReconcileTorrents(state);
            }
        });
}

void Sessions::ReconcileTorrents(const SessionStatePtr& state)
{
    state->m_reconcile_pending = false;

    std::vector<lt::torrent_handle> handles;

    try
    {
        handles = state->session->get_torrents();
    }
    catch(const std::exception& e)
    {
        BOOST_LOG_TRIVIAL(error) << Sub(state) << "Failed to list torrents to reconcile: " << e.what();
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

        if (state->torrents.contains(hash))
        {
            continue;
        }

        if (!keys)
        {
            keys.emplace();

            for (const auto& [ h, s ] : state->torrents)
            {
                keys->emplace(s.handle, h);
            }
        }

        if (const auto key = keys->find(th); key != keys->end())
        {
            UpdateInfoHashes(state, key->second, hash);

            try
            {
                const auto params = th.get_resume_data(lt::torrent_handle::save_info_dict);

                AddTorrentParams::Update(
                    m_options.db,
                    state->id,
                    hash,
                    params,
                    th.userdata().get<TorrentClientData>(),
                    static_cast<int>(th.queue_position()));

                if (params.ti)
                {
                    AddTorrentParams::InsertTorrentInfo(
                        m_options.db,
                        state->id,
                        hash,
                        *params.ti);
                }
            }
            catch(const std::exception& e)
            {
                BOOST_LOG_TRIVIAL(error) << Sub(state, hash) << "Failed to store torrent with updated hashes: " << e.what();
            }

            rehashed++;

            continue;
        }

        const auto status = th.status();

        state->torrents.emplace(hash, status);

        if (state->m_adding.erase(hash) > 0)
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
                state->id,
                hash,
                params,
                data == nullptr ? TorrentClientData{} : *data,
                static_cast<int>(status.queue_position));

            if (params.ti)
            {
                AddTorrentParams::InsertTorrentInfo(
                    m_options.db,
                    state->id,
                    hash,
                    *params.ti);
            }
        }
        catch (const std::exception& e)
        {
            BOOST_LOG_TRIVIAL(error) << Sub(state, hash) << "Failed to insert reconciled torrent: " << e.what();
        }

        if (m_options.events.HasSubscribers("torrent.added"))
        {
            nlohmann::json event_data = nlohmann::json::object();
            event_data["error"] = lt::error_code{};

            TorrentEvent added("torrent.added", std::move(event_data));
            added.torrent_handle = th;
            added.info_hash      = hash;

            Publish(state, std::move(added));
        }

        tracked++;
    }

    if (state->torrents.size() != handles.size())
    {
        std::vector<lt::info_hash_t> gone;

        for (const auto& [ hash, _ ] : state->torrents)
        {
            if (!live.contains(hash))
            {
                gone.push_back(hash);
            }
        }

        for (const auto& hash : gone)
        {
            UntrackTorrent(state, hash);
            untracked++;
        }
    }

    if (tracked + loaded + rehashed + untracked == 0)
    {
        return;
    }

    BOOST_LOG_TRIVIAL(warning)
        << Sub(state) << "Reconciled torrents:"
        << " tracked=" << tracked
        << " loaded=" << loaded
        << " rehashed=" << rehashed
        << " untracked=" << untracked;
}
