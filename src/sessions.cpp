#include "sessions.hpp"

#include <filesystem>
#include <fstream>

#include <boost/log/trivial.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/extensions/ut_metadata.hpp>
#include <libtorrent/extensions/ut_pex.hpp>
#include <libtorrent/extensions/smart_ban.hpp>

#include "data/models/addtorrentparams.hpp"
#include "data/models/sessions.hpp"
#include "torrentclientdata.hpp"

namespace
{
    static constexpr int kLoadChunkSize      = 100;
    static constexpr int  kLoadMaxChunkErrors = 8;
    static constexpr auto kLoadRetryBaseDelay = std::chrono::milliseconds(100);
    static constexpr auto kLoadRetryMaxDelay  = std::chrono::milliseconds(5000);
}

namespace fs = std::filesystem;

using porla::Data::Models::AddTorrentParams;
using porla::Sessions;
using porla::SessionsOptions;

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

void Sessions::SessionState::Recheck(const lt::info_hash_t& hash)
{
    const auto it = torrents.find(hash);

    if (it == torrents.end())
    {
        BOOST_LOG_TRIVIAL(warning)
            << "session[" << name << "][" << hash << "] Failed to find torrent for rechecking";
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
    BOOST_LOG_TRIVIAL(info) << "Shutting down sessions";

    std::vector<SessionStatePtr> pending;
    pending.reserve(m_sessions.size());

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
            BOOST_LOG_TRIVIAL(error) << "session[" << state->name << "] Failed to unload: " << e.what();
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
    auto s = Data::Models::Sessions::GetById(m_options.db, id);

    if (!s)
    {
        BOOST_LOG_TRIVIAL(warning) << "No session with id " << id;
        if (callback) { callback(); }
        return;
    }

    auto session = s.value();

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

    state->m_timers.emplace_back(m_options.io, session.timer_dht_stats, [this, state] { PostDhtStats(state); });
    state->m_timers.emplace_back(m_options.io, session.timer_save_state, [this, state] { SaveState(state); });
    state->m_timers.emplace_back(m_options.io, session.timer_session_stats, [this, state] { PostSessionStats(state); });
    state->m_timers.emplace_back(m_options.io, session.timer_torrent_updates, [this, state] { PostTorrentUpdates(state); });

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
        << "session[" << state->name << "] Loading " << state->m_load_state->count
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
        BOOST_LOG_TRIVIAL(error)
            << "session[" << state->name << "] Failed to read alerts during load: " << e.what();
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
                << "session[" << state->name << "] Chunk failed at " << load.loaded
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
            << "session[" << state->name << "] Failed to load torrents after "
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
                    [](const auto& ts) { return true; });

                for (const auto& ts : all_statuses)
                {
                    state->torrents.insert_or_assign(ts.info_hashes, ts);
                }
            }
            catch(const std::exception& e)
            {
                BOOST_LOG_TRIVIAL(error)
                    << "session[" << state->name << "] Failed to read torrent status after load: "
                    << e.what();
            }
        }

        if (load.count > 0)
        {
            if (load.failed)
            {
                const auto remaining = load.count - load.loaded;

                BOOST_LOG_TRIVIAL(error)
                    << "session[" << state->name << "] Incomplete load - "
                    << load.loaded << " of " << load.count
                    << " torrent(s) were added. The remaining "
                    << remaining << " are still in the database but not in the session. "
                    << "Do not re-add them; restart Porla once the database is healthy.";
            }
            else
            {
                BOOST_LOG_TRIVIAL(info)
                    << "session[" << state->name << "] Added " << load.loaded
                    << " (of " << load.count << ") torrent(s) to the session";
            }
        }

        FinishLoad(state);

        return;
    }

    load.chunks++;

    if (load.chunks % 10 == 0)
    {
        BOOST_LOG_TRIVIAL(info)
            << "session[" << state->name << "] " << load.loaded << " torrents (of "
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
            << "session[" << state->name << "] Load completion callback failed: "
            << e.what();
    }
    
}

void Sessions::UnloadById(int id)
{
    const auto it = m_sessions.find(id);

    if (it == m_sessions.end())
    {
        return;
    }

    UnloadSession(it->second);

    m_sessions.erase(it);
}

void Sessions::ReadAlerts(const std::shared_ptr<SessionState>& state)
{
    std::vector<lt::alert*> alerts;
    state->session->pop_alerts(&alerts);

    for (const auto alert : alerts)
    {
        try
        {
            ProcessAlert(state, alert);
        }
        catch(const std::exception& e)
        {
            BOOST_LOG_TRIVIAL(error)
                << "session[" << state->name << "] Failed to process " << alert->what() << ": " << e.what();
        }
    }
}

void Sessions::ProcessAlert(const SessionStatePtr& state, const lt::alert* alert)
{
    BOOST_LOG_TRIVIAL(trace) << "session[" << state->name << "] " << alert->what() << ": " << alert->message();

    switch (alert->type())
    {
        case lt::add_torrent_alert::alert_type:      OnAddTorrentAlert(state, lt::alert_cast<lt::add_torrent_alert>(alert));          break;
        case lt::file_error_alert::alert_type:       OnFileErrorAlert(state, lt::alert_cast<lt::file_error_alert>(alert));            break;
        case lt::save_resume_data_alert::alert_type: OnSaveResumeDataAlert(state, lt::alert_cast<lt::save_resume_data_alert>(alert)); break;
        case lt::torrent_removed_alert::alert_type:  OnTorrentRemovedAlert(state, lt::alert_cast<lt::torrent_removed_alert>(alert));  break;
        case lt::torrent_resumed_alert::alert_type:  OnTorrentResumedAlert(state, lt::alert_cast<lt::torrent_resumed_alert>(alert));  break;

        case lt::listen_failed_alert::alert_type:
        {
            const auto lfa = lt::alert_cast<lt::listen_failed_alert>(alert);
            BOOST_LOG_TRIVIAL(warning) << "session[" << state->name << "] " << lfa->message();
            break;
        }
        case lt::listen_succeeded_alert::alert_type:
        {
            const auto lsa = lt::alert_cast<lt::listen_succeeded_alert>(alert);
            BOOST_LOG_TRIVIAL(info) << "session[" << state->name << "] " << lsa->message();
            break;
        }
        case lt::metadata_received_alert::alert_type:
        {
            auto mra = lt::alert_cast<lt::metadata_received_alert>(alert);

            if (!mra->handle.is_valid())
            {
                break;
            }

            const auto it = state->torrents.find(mra->handle.info_hashes());

            if (it != state->torrents.end())
            {
                BOOST_LOG_TRIVIAL(info)
                    << "session[" << state->name << "] Metadata received for torrent " << it->second.name;
            }

            mra->handle.save_resume_data(
                lt::torrent_handle::flush_disk_cache
                | lt::torrent_handle::save_info_dict
                | lt::torrent_handle::only_if_modified);

            break;
        }
        case lt::session_stats_alert::alert_type:
        {
            auto ssa = lt::alert_cast<lt::session_stats_alert>(alert);
            auto const& counters = ssa->counters();

            boost::asio::post(m_options.io, [this, counters, weak = std::weak_ptr(state)]()
            {
                if (auto state = weak.lock()) { m_session_stats(state, counters); }
            });

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
                        << "session[" << state->name << "][" << status.info_hashes << "] State updated for missing torrent";
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

            const auto it = state->torrents.find(sma->handle.info_hashes());

            if (it != state->torrents.end())
            {
                BOOST_LOG_TRIVIAL(info)
                    << "session[" << state->name << "] Torrent " << it->second.name << " moved to " << sma->storage_path();
            }

            if (sma->handle.need_save_resume_data())
            {
                sma->handle.save_resume_data(
                    lt::torrent_handle::flush_disk_cache
                    | lt::torrent_handle::save_info_dict
                    | lt::torrent_handle::only_if_modified);
            }

            sma->handle.post_status();

            boost::asio::post(m_options.io, [this, weak = std::weak_ptr(state), th = sma->handle]()
            {
                if (auto state = weak.lock()) { m_storage_moved(state, th); }
            });

            break;
        }
        case lt::torrent_checked_alert::alert_type:
        {
            const auto tca = lt::alert_cast<lt::torrent_checked_alert>(alert);

            if (!tca->handle.is_valid())
            {
                break;
            }

            const auto it  = state->torrents.find(tca->handle.info_hashes());

            if (it != state->torrents.end())
            {
                BOOST_LOG_TRIVIAL(info)
                    << "session[" << state->name << "] Torrent "
                    << it->second.name << " finished checking";
            }
            else
            {
                BOOST_LOG_TRIVIAL(warning)
                    << "session[" << state->name << "] Checked alert for untracked torrent "
                    << tca->handle.info_hashes();
            }

            if (auto node = state->m_oneshot_torrent_callbacks.extract({ alert->type(), tca->handle.info_hashes() }))
            {
                for (auto&& cb : node.mapped()) { cb(state); }
            }

            break;
        }
        case lt::torrent_finished_alert::alert_type:
        {
            const auto tfa          = lt::alert_cast<lt::torrent_finished_alert>(alert);

            if (!tfa->handle.is_valid())
            {
                break;
            }

            const auto& status      = tfa->handle.status();
                    auto  client_data = tfa->handle.userdata().get<TorrentClientData>();

            if (client_data != nullptr)
            {
                const auto contains_signaled_finished = client_data->metadata.contains("signal:finished");

                const auto has_signaled_finished = contains_signaled_finished
                    && client_data->metadata["signal:finished"] == true;

                // A torrent finished signal should only be emitted once per
                // torrent. If we emit this signal, store it in the torrent metadata.

                if (status.total_download > 0 && !has_signaled_finished)
                {
                    client_data->metadata.insert({ "signal:finished", true });

                    // Only emit this event if we have downloaded any data this session
                    BOOST_LOG_TRIVIAL(info) << "session[" << state->name << "] Torrent " << status.name << " finished";

                    boost::asio::post(
                        m_options.io,
                        [this, weak = std::weak_ptr(state), handle = tfa->handle]()
                        {
                            if (auto state = weak.lock()) { m_torrent_finished(state, handle); }
                        });
                }
            }

            if (bool(status.need_save_resume_data))
            {
                status.handle.save_resume_data(
                    lt::torrent_handle::flush_disk_cache
                    | lt::torrent_handle::save_info_dict
                    | lt::torrent_handle::only_if_modified);
            }

            break;
        }
        case lt::torrent_paused_alert::alert_type:
        {
            const auto tpa = lt::alert_cast<lt::torrent_paused_alert>(alert);

            BOOST_LOG_TRIVIAL(debug)
                << "session[" << state->name << "][" << tpa->handle.info_hashes() << "] paused";

            boost::asio::post(m_options.io, [this, weak = std::weak_ptr(state), th = tpa->handle]()
            {
                if (auto state = weak.lock()) { m_torrent_paused(state, th); }
            });

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
    std::vector<lt::torrent_status> torrents = state->session->get_torrent_status(
        [](lt::torrent_status const& ts)
        {
            return bool(ts.need_save_resume_data);
        });

    if (torrents.empty())
    {
        return;
    }

    BOOST_LOG_TRIVIAL(info) << "session[" << state->name << "] Saving state for " << torrents.size() << " torrent(s) in session " << state->name;

    for (const auto& ts : torrents)
    {
        ts.handle.save_resume_data(
            lt::torrent_handle::flush_disk_cache
            | lt::torrent_handle::save_info_dict
            | lt::torrent_handle::only_if_modified);
    }
}

void Sessions::UnloadSession(const std::shared_ptr<SessionState>& state)
{
    FinishLoad(state);

    state->m_timers.clear();
    state->session->set_alert_notify({});

    if (auto session = Data::Models::Sessions::GetById(m_options.db, state->id))
    {
        session->params = state->session->session_state();
        Data::Models::Sessions::Update(m_options.db, *session);
    }

    state->session->pause();

    auto torrents = state->session->get_torrent_status(
        [](const lt::torrent_status& ts)
        {
            return ts.has_metadata && bool(ts.need_save_resume_data);
        });

    int chunk_size = 1000;
    int chunks     = static_cast<int>(torrents.size() / chunk_size) + 1;

    BOOST_LOG_TRIVIAL(info) << "session[" << state->name << "] "
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
                    | lt::torrent_handle::save_info_dict
                    | lt::torrent_handle::only_if_modified);

                outstanding++;
            }
            catch(const std::exception& e)
            {
                BOOST_LOG_TRIVIAL(warning)
                    << "session[" << state->name << "] Failed to post save resume data: "
                    << e.what();
            }

            std::advance(current, 1);
        }

        BOOST_LOG_TRIVIAL(info) << "session[" << state->name << "] " 
                                << "Chunk " << i + 1 << " - Saving state for " << outstanding
                                << " torrent(s) (out of " << chunk_items << ")";

        while (outstanding > 0) {
            const auto found_alert = state->session->wait_for_alert(lt::seconds(10));
            if (!found_alert) { continue; }

            std::vector<lt::alert *> alerts;
            state->session->pop_alerts(&alerts);

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
                        << "session[" << state->name << "] " 
                        << "Failed to save resume data for "
                        << fail->handle.info_hashes()
                        << ": " << fail->message();

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
                        static_cast<int>(rd->handle.status().queue_position));

                    if (data != nullptr)
                    {
                        AddTorrentParams::UpdateClientData(
                            m_options.db,
                            state->id,
                            info_hash,
                            *data);
                    }
                }
                catch(const std::exception& e)
                {
                    BOOST_LOG_TRIVIAL(error) << "session[" << state->name << "] "
                        << "Failed to save resume data for " << rd->params.info_hashes << ": " << e.what();
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
            << "session[" << state->name << "][" << info_hash << "] "
            << "Failed to add torrent " << name << ": " << alert->error.what();

        state->m_adding.erase(info_hash);

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
            << "session[" << state->name << "] Torrent " << status.name
            << " already in session - ignoring duplicate add";

        return;
    }

    if (data == nullptr)
    {
        BOOST_LOG_TRIVIAL(error)
            << "session[" << state->name << "] Torrent " << status.name << " has missing user data";
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
    }
    catch(const std::exception& e)
    {
        BOOST_LOG_TRIVIAL(error)
            << "session[" << state->name << "] Failed to insert torrent "
            << status.name << ": " << e.what();
    }

    alert->handle.save_resume_data(
        lt::torrent_handle::flush_disk_cache
        | lt::torrent_handle::save_info_dict
        | lt::torrent_handle::only_if_modified);

    Emit(m_torrent_added, state, alert->handle);
}

void Sessions::OnFileErrorAlert(const SessionStatePtr& state, const lt::file_error_alert* alert)
{
    TorrentFileErrorEvent evt{
        .file = alert->filename(),
        .torrent = alert->handle
    };

    Emit(m_torrent_file_error, state, evt);
}

void Sessions::OnSaveResumeDataAlert(const SessionStatePtr& state, const lt::save_resume_data_alert* alert)
{
    if (!alert->handle.is_valid())
    {
        BOOST_LOG_TRIVIAL(debug)
            << "session[" << state->name << "][" << alert->params.info_hashes
            << "] Resume data for invalid torrent - discarding";

        return;
    }

    const auto& status = alert->handle.status();
    const auto  data   = alert->handle.userdata().get<TorrentClientData>();

    AddTorrentParams::Update(
        m_options.db,
        state->id,
        status.info_hashes,
        alert->params,
        static_cast<int>(status.queue_position));

    if (data != nullptr)
    {
        AddTorrentParams::UpdateClientData(
            m_options.db,
            state->id,
            status.info_hashes,
            *data);
    }

    BOOST_LOG_TRIVIAL(debug) << "session[" << state->name << "] Resume data saved for " << status.name;
}

void Sessions::OnTorrentRemovedAlert(const SessionStatePtr& state, const lt::torrent_removed_alert* alert)
{
    const auto it = state->torrents.find(alert->info_hashes);

    if (it == state->torrents.end())
    {
        BOOST_LOG_TRIVIAL(warning)
            << "session[" << state->name << "] Removed alert for untracked torrent " << alert->info_hashes;
    }
    else
    {
        const auto name = it->second.name;
        state->torrents.erase(it);
        BOOST_LOG_TRIVIAL(info) << "session[" << state->name << "] Torrent " << name << " removed";
    }

    std::erase_if(
        state->m_oneshot_torrent_callbacks,
        [&](const auto& kv)
        {
            return kv.first.second == alert->info_hashes;
        });

    Emit(m_torrent_removed, state, alert->info_hashes);

    try
    {
        AddTorrentParams::Remove(m_options.db, state->id, alert->info_hashes);
    }
    catch(const std::exception& e)
    {
        BOOST_LOG_TRIVIAL(error)
            << "session[" << state->name << "] Failed to remove torrent "
            << alert->info_hashes << " from database: " << e.what();
    }

    delete alert->userdata.get<TorrentClientData>();
}

void Sessions::OnTorrentResumedAlert(const SessionStatePtr& state, const lt::torrent_resumed_alert* alert)
{
    const auto it = state->torrents.find(alert->handle.info_hashes());

    if (it == state->torrents.end())
    {
        BOOST_LOG_TRIVIAL(debug)
            << "session[" << state->name << "][" << alert->handle.info_hashes() << "] Unknown torrent resumed";
    }
    else
    {
        BOOST_LOG_TRIVIAL(debug) << "session[" << state->name << "] Torrent " << it->second.name << " resumed";
    }

    Emit(m_torrent_resumed, state, alert->handle);
}
