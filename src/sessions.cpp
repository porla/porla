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

namespace fs = std::filesystem;

using porla::Data::Models::AddTorrentParams;
using porla::Sessions;
using porla::SessionsOptions;

void Sessions::SessionState::Recheck(const lt::info_hash_t& hash)
{
    const auto& [ handle, _ ] = torrents.at(hash);

    // If the torrent is paused, it must be resumed in order to be rechecked.
    // It should also not be auto managed, so remove it from that as well.
    // When the session posts a torrent_check alert, restore its flags.

    bool was_auto_managed = false;
    bool was_paused = false;
    int alert_type = lt::torrent_checked_alert::alert_type;

    if ((handle.flags() & lt::torrent_flags::auto_managed) == lt::torrent_flags::auto_managed)
    {
        handle.unset_flags(lt::torrent_flags::auto_managed);
        was_auto_managed = true;
    }

    if ((handle.flags() & lt::torrent_flags::paused) == lt::torrent_flags::paused)
    {
        handle.resume();
        was_paused = true;
    }

    if (!m_oneshot_torrent_callbacks.contains({ alert_type, hash }))
    {
        m_oneshot_torrent_callbacks.insert({{ alert_type, hash }, {}});
    }

    m_oneshot_torrent_callbacks.at({ alert_type, hash }).emplace_back(
        [&, hash, was_auto_managed, was_paused]()
        {
            if (!torrents.contains(hash))
            {
                return;
            }

            const auto& [ th, _ ] = torrents.at(hash);

            // TODO: Unsure about the order here. If there are reports that force-checking a torrent
            //       leads to any issues with resume/pause, the order of these statements might matter.

            if (was_auto_managed)
            {
                th.set_flags(lt::torrent_flags::auto_managed);
            }

            if (was_paused)
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

    for (const auto& [ _, state ] : m_sessions)
    {
        UnloadSession(state);
    }

    BOOST_LOG_TRIVIAL(info) << "All state saved";
}

std::shared_ptr<Sessions::SessionState> Sessions::Get(const int id)
{
    if (m_sessions.contains(id))
    {
        return m_sessions.at(id);
    }

    return nullptr;
}

void Sessions::LoadAll()
{
    const auto& sessions = Data::Models::Sessions::List(m_options.db);

    BOOST_LOG_TRIVIAL(info) << "Loading " << sessions.size() << " session(s)";

    for (const auto& session : sessions)
    {
        LoadById(session.id);
    }
}

void Sessions::LoadById(int id)
{
    auto s = Data::Models::Sessions::GetById(m_options.db, id);

    if (!s)
    {
        BOOST_LOG_TRIVIAL(warning) << "No session with id " << id;
        return;
    }

    auto session = s.value();

    if (m_sessions.contains(session.id))
    {
        BOOST_LOG_TRIVIAL(warning) << "session[" << session.name << "] Already loaded - skipping";
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
        [this, state]()
        {
            boost::asio::post(m_options.io, [this, state] { ReadAlerts(state); });
        });

    state->m_timers.emplace_back(m_options.io, session.timer_dht_stats, [this, state] { PostDhtStats(state); });
    state->m_timers.emplace_back(m_options.io, session.timer_save_state, [this, state] { SaveState(state); });
    state->m_timers.emplace_back(m_options.io, session.timer_session_stats, [this, state] { PostSessionStats(state); });
    state->m_timers.emplace_back(m_options.io, session.timer_torrent_updates, [this, state] { PostTorrentUpdates(state); });

    int count = AddTorrentParams::Count(m_options.db, session.id);
    int current = 0;

    BOOST_LOG_TRIVIAL(info) << "session[" << state->name << "] Loading " << count << " torrent(s) from storage";

    AddTorrentParams::ForEach(
        m_options.db,
        state->id,
        [&count, &current, state](lt::add_torrent_params& params)
        {
            current++;

            params.userdata.get<TorrentClientData>()->state = state;

            state->m_adding.insert(params.info_hashes);
            state->session->async_add_torrent(params);

            if (current % 1000 == 0 && current != count)
            {
                BOOST_LOG_TRIVIAL(info) << "session[" << state->name << "] " << current << " torrents (of " << count << ") added";
            }
        });

    if (count > 0)
    {
        BOOST_LOG_TRIVIAL(info) << "session[" << state->name << "] Added " << current << " (of " << count << ") torrent(s) to session";
    }

    m_sessions.insert({ state->id, state });
}

void Sessions::UnloadById(int id)
{
    if (!m_sessions.contains(id))
    {
        return;
    }

    UnloadSession(m_sessions.at(id));

    m_sessions.erase(id);
}

void Sessions::ReadAlerts(const std::shared_ptr<SessionState>& state)
{
    std::vector<lt::alert*> alerts;
    state->session->pop_alerts(&alerts);

    for (const auto alert : alerts)
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

                BOOST_LOG_TRIVIAL(info) << "session[" << state->name << "] Metadata received for torrent " << mra->handle.status().name;

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

                boost::asio::post(m_options.io, [this, counters, state](){ m_session_stats(state, counters); });

                break;
            }
            case lt::state_update_alert::alert_type:
            {
                auto sua = lt::alert_cast<lt::state_update_alert>(alert);

                for (const auto& status : sua->status)
                {
                    state->torrents.at(status.info_hashes) = std::make_pair(status.handle, status);
                }

                boost::asio::post(m_options.io, [this, state, status = sua->status](){ m_state_update(state, status); });

                break;
            }
            case lt::storage_moved_alert::alert_type:
            {
                const auto sma = lt::alert_cast<lt::storage_moved_alert>(alert);
                const auto name = sma->handle.status(lt::torrent_handle::query_name).name;

                BOOST_LOG_TRIVIAL(info) << "session[" << state->name << "] Torrent " << name << " moved to " << sma->storage_path();

                if (sma->handle.need_save_resume_data())
                {
                    sma->handle.save_resume_data(
                        lt::torrent_handle::flush_disk_cache
                        | lt::torrent_handle::save_info_dict
                        | lt::torrent_handle::only_if_modified);
                }

                state->torrents.at(sma->handle.info_hashes()) = std::make_pair(sma->handle, sma->handle.status());

                boost::asio::post(m_options.io, [this, state, th = sma->handle](){ m_storage_moved(state, th); });

                break;
            }
            case lt::torrent_checked_alert::alert_type:
            {
                const auto tca = lt::alert_cast<lt::torrent_checked_alert>(alert);
                const auto name = tca->handle.status(lt::torrent_handle::query_name).name;

                BOOST_LOG_TRIVIAL(info) << "session[" << state->name << "] Torrent " << name << " finished checking";

                state->torrents.at(tca->handle.info_hashes()) = std::make_pair(tca->handle, tca->handle.status());

                if (state->m_oneshot_torrent_callbacks.contains({ alert->type(), tca->handle.info_hashes()}))
                {
                    for (auto && cb : state->m_oneshot_torrent_callbacks.at({ alert->type(), tca->handle.info_hashes() }))
                    {
                        cb();
                    }

                    state->m_oneshot_torrent_callbacks.erase({ alert->type(), tca->handle.info_hashes() });
                }

                break;
            }
            case lt::torrent_finished_alert::alert_type:
            {
                const auto tfa          = lt::alert_cast<lt::torrent_finished_alert>(alert);
                const auto& status      = tfa->handle.status();
                      auto  client_data = tfa->handle.userdata().get<TorrentClientData>();

                const auto contains_signaled_finished = client_data->metadata.contains("signal:finished");

                const auto has_signaled_finished = contains_signaled_finished
                    && client_data->metadata["signal:finished"] == true;

                state->torrents.at(tfa->handle.info_hashes()) = std::make_pair(tfa->handle, status);

                // A torrent finished signal should only be emitted once per
                // torrent. If we emit this signal, store it in the torrent metadata.

                if (status.total_download > 0 && !has_signaled_finished)
                {
                    client_data->metadata.insert({ "signal:finished", true });

                    // Only emit this event if we have downloaded any data this session
                    BOOST_LOG_TRIVIAL(info) << "session[" << state->name << "] Torrent " << status.name << " finished";

                    boost::asio::post(
                        m_options.io,
                        [this, state, handle = tfa->handle]()
                        {
                            m_torrent_finished(state, handle);
                        });
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
                const auto name = tpa->handle.status(lt::torrent_handle::query_name).name;

                BOOST_LOG_TRIVIAL(debug) << "session[" << state->name << "] Torrent " << name << " paused";

                state->torrents.at(tpa->handle.info_hashes()) = std::make_pair(tpa->handle, tpa->handle.status());

                boost::asio::post(m_options.io, [this, state, th = tpa->handle](){ m_torrent_paused(state, th); });

                break;
            }
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
    state->m_timers.clear();
    state->session->set_alert_notify({});

    if (auto session = Data::Models::Sessions::GetById(m_options.db, state->id))
    {
        session->params = state->session->session_state();
        Data::Models::Sessions::Update(m_options.db, *session);
    }

    state->session->pause();

    int chunk_size = 1000;
    int chunks = static_cast<int>(state->torrents.size() / chunk_size) + 1;

    BOOST_LOG_TRIVIAL(info) << "session[" << state->name << "] "
                            << "Saving resume data in " << chunks << " chunk(s) - total torrents: "
                            << state->torrents.size();

    auto current = state->torrents.begin();

    for (int i = 0; i < chunks; i++)
    {
        int chunk_items = std::min(
            chunk_size,
            static_cast<int>(std::distance(current, state->torrents.end())));

        int outstanding = 0;

        for (int j = 0; j < chunk_items; j++)
        {
            const auto& [ th, ts ] = current->second;

            if (!th.is_valid() || !ts.has_metadata || !bool(ts.need_save_resume_data))
            {
                std::advance(current, 1);
                continue;
            }

            th.save_resume_data(
                lt::torrent_handle::flush_disk_cache
                | lt::torrent_handle::save_info_dict
                | lt::torrent_handle::only_if_modified);

            outstanding++;

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
                    const auto name = fail->handle.status(lt::torrent_handle::query_name).name;

                    outstanding--;

                    BOOST_LOG_TRIVIAL(error)
                        << "session[" << state->name << "] " 
                        << "Failed to save resume data for "
                        << name
                        << ": " << fail->message();

                    continue;
                }

                auto *rd = lt::alert_cast<lt::save_resume_data_alert>(a);
                if (!rd) { continue; }

                outstanding--;

                AddTorrentParams::Update(
                    m_options.db,
                    state->id,
                    rd->handle.info_hashes(),
                    rd->params,
                    rd->handle.userdata().get<TorrentClientData>(),
                    static_cast<int>(rd->handle.status().queue_position));
            }
        }
    }
}

void Sessions::OnAddTorrentAlert(const SessionStatePtr& state, const lt::add_torrent_alert* alert)
{
    const auto name = alert->handle.status(lt::torrent_handle::query_name).name;

    if (alert->error)
    {
        BOOST_LOG_TRIVIAL(error) << "session[" << state->name << "] Failed to add torrent " << name << ": " << alert->error.what();
        return;
    }

    const auto status = alert->handle.status();

    state->torrents.insert({
        alert->handle.info_hashes(),
        std::make_pair(alert->handle, status) });

    if (state->m_adding.contains(alert->handle.info_hashes()))
    {
        state->m_adding.erase(alert->handle.info_hashes());
        return;
    }

    AddTorrentParams::Insert(
        m_options.db,
        state->id,
        alert->handle.info_hashes(),
        alert->params,
        alert->handle.userdata().get<TorrentClientData>(),
        static_cast<int>(status.queue_position));

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
    const auto& status = alert->handle.status();

    AddTorrentParams::Update(
        m_options.db,
        state->id,
        status.info_hashes,
        alert->params,
        alert->handle.userdata().get<TorrentClientData>(),
        static_cast<int>(status.queue_position));

    BOOST_LOG_TRIVIAL(debug) << "session[" << state->name << "] Resume data saved for " << status.name;
}

void Sessions::OnTorrentRemovedAlert(const SessionStatePtr& state, const lt::torrent_removed_alert* alert)
{
    AddTorrentParams::Remove(m_options.db, state->id, alert->info_hashes);

    const auto [ _, ts ] = state->torrents.at(alert->info_hashes);

    state->torrents.erase(alert->info_hashes);

    BOOST_LOG_TRIVIAL(info) << "session[" << state->name << "] Torrent " << ts.name << " removed";

    Emit(m_torrent_removed, state, alert->info_hashes);
}

void Sessions::OnTorrentResumedAlert(const SessionStatePtr& state, const lt::torrent_resumed_alert* alert)
{
    const auto& status = alert->handle.status();

    BOOST_LOG_TRIVIAL(debug) << "session[" << state->name << "] Torrent " << status.name << " resumed";

    state->torrents.at(alert->handle.info_hashes()) = std::make_pair(alert->handle, status);

    Emit(m_torrent_resumed, state, alert->handle);
}
