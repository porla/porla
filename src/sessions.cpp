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

class Sessions::Timer
{
public:
    explicit Timer(boost::asio::io_context& io, int interval, std::function<void()> cb)
        : m_timer(io)
        , m_interval(interval)
        , m_callback(std::move(cb))
    {
        boost::system::error_code ec;

        m_timer.expires_from_now(boost::posix_time::milliseconds(m_interval), ec);
        if (ec) { BOOST_LOG_TRIVIAL(error) << "Failed to set timer expiry: " << ec.message(); }

        m_timer.async_wait([this](auto &&PH1) { OnExpired(std::forward<decltype(PH1)>(PH1)); });
    }

    Timer(Timer&& t) noexcept
        : m_timer(std::move(t.m_timer))
        , m_interval(std::exchange(t.m_interval, 0))
        , m_callback(std::move(t.m_callback))
    {
        boost::system::error_code ec;

        m_timer.cancel(ec);
        if (ec) { BOOST_LOG_TRIVIAL(error) << "Failed to cancel timer: " << ec.message(); }

        m_timer.expires_from_now(boost::posix_time::milliseconds(m_interval), ec);
        if (ec) { BOOST_LOG_TRIVIAL(error) << "Failed to set timer expiry: " << ec.message(); }

        m_timer.async_wait([this](auto &&PH1) { OnExpired(std::forward<decltype(PH1)>(PH1)); });
    }

    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;
    Timer& operator=(Timer&&) = delete; // noexcept {}

private:
    void OnExpired(boost::system::error_code ec)
    {
        if (ec == boost::system::errc::operation_canceled)
        {
            return;
        }
        else if (ec)
        {
            BOOST_LOG_TRIVIAL(error) << "Error in timer: " << ec.message();
            return;
        }

        m_callback();

        m_timer.expires_from_now(boost::posix_time::milliseconds(m_interval), ec);
        if (ec) { BOOST_LOG_TRIVIAL(error) << "Failed to set timer expiry: " << ec; }

        m_timer.async_wait([this](auto &&PH1) { OnExpired(std::forward<decltype(PH1)>(PH1)); });
    }

    boost::asio::deadline_timer m_timer;
    int m_interval;
    std::function<void()> m_callback;
};

static lt::session_params ReadSessionParams(const fs::path& file)
{
    if (fs::exists(file))
    {
        std::ifstream session_params_file(file, std::ios::binary);

        // Get the params file size
        session_params_file.seekg(0, std::ios_base::end);
        const std::streamsize session_params_size = session_params_file.tellg();
        session_params_file.seekg(0, std::ios_base::beg);

        BOOST_LOG_TRIVIAL(info) << "Reading session params (" << session_params_size << " bytes)";

        // Create a buffer to hold the contents of the session params file
        std::vector<char> session_params_buffer;
        session_params_buffer.resize(session_params_size);

        // Actually read the file
        session_params_file.read(session_params_buffer.data(), session_params_size);

        // Only load the DHT state from the session params. Settings are stored elsewhere.
        return lt::read_session_params(session_params_buffer, lt::session::save_dht_state);
    }

    return {};
}

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
    if (options.timer_dht_stats > 0)
        m_timers.emplace_back(options.io, options.timer_dht_stats, [&] { PostDhtStats(); });

    if (options.timer_save_state > 0)
        m_timers.emplace_back(options.io, options.timer_save_state, [&] { SaveState(); });

    if (options.timer_session_stats > 0)
        m_timers.emplace_back(options.io, options.timer_session_stats, [&] { PostSessionStats(); });

    if (options.timer_torrent_updates > 0)
        m_timers.emplace_back(options.io, options.timer_torrent_updates, [&] { PostTorrentUpdates(); });

}

Sessions::~Sessions()
{
    BOOST_LOG_TRIVIAL(info) << "Shutting down sessions";

    m_timers.clear();

    for (const auto& [ name, state ] : m_sessions)
    {
        UnloadSession(name, state);
    }

    BOOST_LOG_TRIVIAL(info) << "All state saved";
}

std::map<std::string, std::shared_ptr<Sessions::SessionState>>& Sessions::All()
{
    return m_sessions;
}

std::shared_ptr<Sessions::SessionState> Sessions::Default()
{
    return m_sessions.at("default");
}

std::shared_ptr<Sessions::SessionState> Sessions::Get(const std::string& name)
{
    return m_sessions.at(name);
}


void Sessions::LoadAll()
{
    std::vector<Data::Models::Sessions::Session> items;

    Data::Models::Sessions::ForEach(
        m_options.db,
        [&items](const Data::Models::Sessions::Session& s)
        {
            items.push_back(s);
        });

    BOOST_LOG_TRIVIAL(info) << "Loading " << items.size() << " sessions";

    for (const auto& session : items)
    {
        if (m_sessions.contains(session.name))
        {
            BOOST_LOG_TRIVIAL(warning) << "Session " << session.name << " already loaded";
            continue;
        }

        auto state = std::make_shared<SessionState>();
        state->name = session.name;
        state->session = std::make_shared<lt::session>(std::move(session.params));
        state->session->add_extension(&lt::create_ut_metadata_plugin);
        state->session->add_extension(&lt::create_ut_pex_plugin);
        state->session->add_extension(&lt::create_smart_ban_plugin);
        state->torrents = {};

        state->session->set_alert_notify(
            [this, state]()
            {
                boost::asio::post(m_options.io, [this, state] { ReadAlerts(state); });
            });

        int count = AddTorrentParams::Count(m_options.db, session.name);
        int current = 0;

        BOOST_LOG_TRIVIAL(info) << "Loading " << count << " torrent(s) from storage";

        AddTorrentParams::ForEach(
            m_options.db,
            state->name,
            [&count, &current, state](lt::add_torrent_params& params)
            {
                current++;

                params.userdata.get<TorrentClientData>()->ignore_alert = true;
                params.userdata.get<TorrentClientData>()->state = state;

                lt::torrent_handle th = state->session->add_torrent(params);
                state->torrents.insert({ th.info_hashes(), std::make_pair(th, th.status()) });

                if (current % 1000 == 0 && current != count)
                {
                    BOOST_LOG_TRIVIAL(info) << current << " torrents (of " << count << ") added";
                }
            });

        if (count > 0)
        {
            BOOST_LOG_TRIVIAL(info) << "Added " << current << " (of " << count << ") torrent(s) to session";
        }

        m_sessions.insert({ session.name, state });
    }
}

void Sessions::LoadByName(const std::string& name)
{
    auto s = Data::Models::Sessions::GetByName(m_options.db, name);

    if (!s)
    {
        BOOST_LOG_TRIVIAL(warning) << "No session with name " << name;
        return;
    }

    auto session = s.value();

    if (m_sessions.contains(session.name))
    {
        BOOST_LOG_TRIVIAL(warning) << "Session " << session.name << " already loaded";
        return;
    }



    auto state = std::make_shared<SessionState>();
    state->name = session.name;
    state->session = std::make_shared<lt::session>(std::move(session.params));
    state->session->add_extension(&lt::create_ut_metadata_plugin);
    state->session->add_extension(&lt::create_ut_pex_plugin);
    state->session->add_extension(&lt::create_smart_ban_plugin);
    state->torrents = {};

    state->session->set_alert_notify(
        [this, state]()
        {
            boost::asio::post(m_options.io, [this, state] { ReadAlerts(state); });
        });

    int count = AddTorrentParams::Count(m_options.db, session.name);
    int current = 0;

    BOOST_LOG_TRIVIAL(info) << "Loading " << count << " torrent(s) from storage";

    AddTorrentParams::ForEach(
        m_options.db,
        state->name,
        [&count, &current, state](lt::add_torrent_params& params)
        {
            current++;

            params.userdata.get<TorrentClientData>()->ignore_alert = true;
            params.userdata.get<TorrentClientData>()->state = state;

            lt::torrent_handle th = state->session->add_torrent(params);
            state->torrents.insert({ th.info_hashes(), std::make_pair(th, th.status()) });

            if (current % 1000 == 0 && current != count)
            {
                BOOST_LOG_TRIVIAL(info) << current << " torrents (of " << count << ") added";
            }
        });

    if (count > 0)
    {
        BOOST_LOG_TRIVIAL(info) << "Added " << current << " (of " << count << ") torrent(s) to session";
    }

    m_sessions.insert({ session.name, state });
}

void Sessions::UnloadByName(const std::string& name)
{
    auto state = m_sessions.find(name);

    if (state != m_sessions.end())
    {
        UnloadSession(state->first, state->second);
    }

    m_sessions.erase(name);
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
            case lt::add_torrent_alert::alert_type:
            {
                const auto ata = lt::alert_cast<lt::add_torrent_alert>(alert);

                if (ata->error)
                {
                    BOOST_LOG_TRIVIAL(error) << "Failed to add torrent " << ata->torrent_name() << ": " << ata->error.what();
                    continue;
                }

                const auto client_data = ata->handle.userdata().get<TorrentClientData>();

                if (client_data != nullptr && client_data->ignore_alert)
                {
                    continue;
                }

                state->torrents.insert({
                    ata->handle.info_hashes(),
                    std::make_pair(ata->handle, ata->handle.status()) });

                const auto status = ata->handle.status();

                AddTorrentParams::Insert(m_options.db, state->name, ata->handle.info_hashes(), AddTorrentParams{
                    .client_data    = ata->handle.userdata().get<TorrentClientData>(),
                    .name           = status.name,
                    .params         = ata->params,
                    .queue_position = static_cast<int>(status.queue_position),
                    .save_path      = status.save_path,
                });

                ata->handle.save_resume_data(
                    lt::torrent_handle::flush_disk_cache
                    | lt::torrent_handle::save_info_dict
                    | lt::torrent_handle::only_if_modified);

                boost::asio::post(
                    m_options.io,
                    [this, state, handle = ata->handle]()
                    {
                        m_torrent_added(state->name, handle);
                    });

                break;
            }
            case lt::file_error_alert::alert_type:
            {
                const auto fea = lt::alert_cast<lt::file_error_alert>(alert);

                TorrentFileErrorEvent evt{
                    .file = fea->filename(),
                    .torrent = fea->handle
                };

                boost::asio::post(m_options.io, [this, evt, state](){ m_torrent_file_error(state->name, evt); });

                break;
            }
            case lt::metadata_received_alert::alert_type:
            {
                auto mra = lt::alert_cast<lt::metadata_received_alert>(alert);

                BOOST_LOG_TRIVIAL(info) << "Metadata received for torrent " << mra->handle.status().name;

                mra->handle.save_resume_data(
                    lt::torrent_handle::flush_disk_cache
                    | lt::torrent_handle::save_info_dict
                    | lt::torrent_handle::only_if_modified);

                break;
            }
            case lt::save_resume_data_alert::alert_type:
            {
                auto srda = lt::alert_cast<lt::save_resume_data_alert>(alert);
                auto const& status = srda->handle.status();

                AddTorrentParams::Update(m_options.db, state->name, status.info_hashes, AddTorrentParams{
                    .client_data    = srda->handle.userdata().get<TorrentClientData>(),
                    .name           = status.name,
                    .params         = srda->params,
                    .queue_position = static_cast<int>(status.queue_position),
                    .save_path      = status.save_path
                });

                BOOST_LOG_TRIVIAL(debug) << "Resume data saved for " << status.name;

                break;
            }
            case lt::session_stats_alert::alert_type:
            {
                auto ssa = lt::alert_cast<lt::session_stats_alert>(alert);
                auto const& counters = ssa->counters();

                boost::asio::post(m_options.io, [this, counters, state](){ m_session_stats(state->name, counters); });

                break;
            }
            case lt::state_update_alert::alert_type:
            {
                auto sua = lt::alert_cast<lt::state_update_alert>(alert);

                for (const auto& status : sua->status)
                {
                    state->torrents.at(status.info_hashes) = std::make_pair(status.handle, status);
                }

                boost::asio::post(m_options.io, [this, state, status = sua->status](){ m_state_update(state->name, status); });

                break;
            }
            case lt::storage_moved_alert::alert_type:
            {
                auto const sma = lt::alert_cast<lt::storage_moved_alert>(alert);

                BOOST_LOG_TRIVIAL(info) << "Torrent " << sma->torrent_name() << " moved to " << sma->storage_path();

                if (sma->handle.need_save_resume_data())
                {
                    sma->handle.save_resume_data(
                        lt::torrent_handle::flush_disk_cache
                        | lt::torrent_handle::save_info_dict
                        | lt::torrent_handle::only_if_modified);
                }

                state->torrents.at(sma->handle.info_hashes()) = std::make_pair(sma->handle, sma->handle.status());

                boost::asio::post(m_options.io, [this, state, th = sma->handle](){ m_storage_moved(state->name, th); });

                break;
            }
            case lt::torrent_checked_alert::alert_type:
            {
                const auto tca = lt::alert_cast<lt::torrent_checked_alert>(alert);

                BOOST_LOG_TRIVIAL(info) << "Torrent " << tca->torrent_name() << " finished checking";

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

                const auto contains_signaled_finished = client_data->metadata.value_or(
                    std::map<std::string, nlohmann::json>()).contains("signal:finished");

                const auto has_signaled_finished = contains_signaled_finished
                    && client_data->metadata.value()["signal:finished"] == true;

                state->torrents.at(tfa->handle.info_hashes()) = std::make_pair(tfa->handle, status);

                // A torrent finished signal should only be emitted once per
                // torrent. If we emit this signal, store it in the torrent metadata.

                if (status.total_download > 0 && !has_signaled_finished)
                {
                    if (!client_data->metadata)
                    {
                        client_data->metadata = std::map<std::string, nlohmann::json>();
                    }

                    client_data->metadata->insert({ "signal:finished", true });

                    // Only emit this event if we have downloaded any data this session
                    BOOST_LOG_TRIVIAL(info) << "Torrent " << status.name << " finished";

                    boost::asio::post(
                        m_options.io,
                        [this, state, handle = tfa->handle]()
                        {
                            m_torrent_finished(state->name, handle);
                        });
                }

                if (status.need_save_resume)
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
                auto tpa = lt::alert_cast<lt::torrent_paused_alert>(alert);

                BOOST_LOG_TRIVIAL(debug) << "Torrent " << tpa->torrent_name() << " paused";

                state->torrents.at(tpa->handle.info_hashes()) = std::make_pair(tpa->handle, tpa->handle.status());

                boost::asio::post(m_options.io, [this, state, th = tpa->handle](){ m_torrent_paused(state->name, th); });

                break;
            }
            case lt::torrent_removed_alert::alert_type:
            {
                auto tra = lt::alert_cast<lt::torrent_removed_alert>(alert);

                AddTorrentParams::Remove(m_options.db, state->name, tra->info_hashes);

                state->torrents.erase(tra->info_hashes);

                BOOST_LOG_TRIVIAL(info) << "Torrent " << tra->torrent_name() << " removed";

                boost::asio::post(m_options.io, [this, hash = tra->info_hashes, state](){ m_torrent_removed(state->name, hash); });

                break;
            }
            case lt::torrent_resumed_alert::alert_type:
            {
                auto tra = lt::alert_cast<lt::torrent_resumed_alert>(alert);
                auto const& status = tra->handle.status();

                BOOST_LOG_TRIVIAL(debug) << "Torrent " << status.name << " resumed";

                state->torrents.at(tra->handle.info_hashes()) = std::make_pair(tra->handle, status);

                boost::asio::post(
                    m_options.io,
                    [this, state, handle = tra->handle]()
                    {
                        m_torrent_resumed(state->name, handle);
                    });

                break;
            }
        }
    }
}

void Sessions::PostDhtStats()
{
    for (const auto &state: m_sessions | std::views::values)
    {
        state->session->post_dht_stats();
    }
}

void Sessions::PostSessionStats()
{
    for (const auto &state: m_sessions | std::views::values)
    {
        state->session->post_session_stats();
    }
}

void Sessions::PostTorrentUpdates()
{
    for (const auto &state: m_sessions | std::views::values)
    {
        state->session->post_torrent_updates();
    }
}

void Sessions::SaveState()
{
    for (const auto &state: m_sessions | std::views::values)
    {
        std::vector<lt::torrent_status> torrents = state->session->get_torrent_status(
            [](lt::torrent_status const& ts)
            {
                return ts.need_save_resume;
            });

        if (torrents.empty())
        {
            continue;
        }

        BOOST_LOG_TRIVIAL(info) << "Saving state for " << torrents.size() << " torrent(s) in session " << state->name;

        for (const auto& ts : torrents)
        {
            ts.handle.save_resume_data(
                lt::torrent_handle::flush_disk_cache
                | lt::torrent_handle::save_info_dict
                | lt::torrent_handle::only_if_modified);
        }
    }
}

void Sessions::UnloadSession(const std::string& name, const std::shared_ptr<SessionState>& state)
{
    state->session->set_alert_notify([]{});

    Data::Models::Sessions::Update(
        m_options.db,
        name,
        state->session->session_state());

    state->session->pause();

    int chunk_size = 1000;
    int chunks = static_cast<int>(state->torrents.size() / chunk_size) + 1;

    BOOST_LOG_TRIVIAL(info) << "Saving resume data in " << chunks << " chunk(s) - total torrents: "
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

            if (!th.is_valid() || !ts.has_metadata || !ts.need_save_resume)
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

        BOOST_LOG_TRIVIAL(info) << "Chunk " << i + 1 << " - Saving state for " << outstanding
                                << " torrent(s) (out of " << chunk_items << ")";

        while (outstanding > 0) {
            lt::alert const *tmp = state->session->wait_for_alert(lt::seconds(10));
            if (tmp == nullptr) { continue; }

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
                        << "Failed to save resume data for "
                        << fail->torrent_name()
                        << ": " << fail->message();

                    continue;
                }

                auto *rd = lt::alert_cast<lt::save_resume_data_alert>(a);
                if (!rd) { continue; }

                outstanding--;

                AddTorrentParams::Update(m_options.db, state->name, rd->handle.info_hashes(), AddTorrentParams{
                    .client_data    = rd->handle.userdata().get<TorrentClientData>(),
                    .name           = rd->params.name,
                    .params         = rd->params,
                    .queue_position = static_cast<int>(rd->handle.status().queue_position),
                    .save_path      = rd->params.save_path
                });
            }
        }
    }
}
