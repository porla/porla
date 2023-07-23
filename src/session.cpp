#include "session.hpp"

#include <fstream>
#include <utility>

#include <boost/log/trivial.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/extensions/ut_metadata.hpp>
#include <libtorrent/extensions/ut_pex.hpp>
#include <libtorrent/extensions/smart_ban.hpp>
#include <libtorrent/session_stats.hpp>

#include "data/models/addtorrentparams.hpp"
#include "torrentclientdata.hpp"

namespace fs = std::filesystem;
namespace lt = libtorrent;

using porla::Data::Models::AddTorrentParams;
using porla::Session;

template<typename T>
static std::string ToString(const T &hash)
{
    std::stringstream ss;
    ss << hash;
    return ss.str();
}

class Session::Timer
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

        // Only load the DHT state from the session params. Settings are stored in our database.
        return lt::read_session_params(session_params_buffer, lt::session::save_dht_state);
    }

    return {};
}

static void WriteSessionParams(const fs::path& file, const lt::session_params& params)
{
    std::vector<char> buf = lt::write_session_params_buf(
        params,
        lt::session::save_dht_state);

    BOOST_LOG_TRIVIAL(info) << "Writing session params (" << buf.size() << " bytes)";

    std::ofstream session_params_file(file, std::ios::binary | std::ios::trunc);

    if (!session_params_file.is_open())
    {
        BOOST_LOG_TRIVIAL(error) << "Error while opening session_params.dat: " << strerror(errno);
        return;
    }

    session_params_file.write(buf.data(), static_cast<std::streamsize>(buf.size()));

    if (session_params_file.fail())
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to write session_params.dat file: " << strerror(errno);
    }
}

Session::Session(boost::asio::io_context& io, porla::SessionOptions const& options)
    : m_io(io)
    , m_db(options.db)
    , m_session_params_file(options.session_params_file)
    , m_stats(lt::session_stats_metrics())
{
    lt::session_params params = ReadSessionParams(m_session_params_file);
    params.settings = options.settings;

    m_session = std::make_unique<lt::session>(std::move(params));

    if (auto extensions = options.extensions)
    {
        BOOST_LOG_TRIVIAL(info) << "Loading " << extensions.value().size() << " user-specified extension(s)";

        for (auto const& extension : extensions.value())
        {
            m_session->add_extension(extension);
        }
    }
    else
    {
        BOOST_LOG_TRIVIAL(info) << "Loading default session extensions (ut_metadata, ut_pex, smart_ban)";

        m_session->add_extension(&lt::create_ut_metadata_plugin);
        m_session->add_extension(&lt::create_ut_pex_plugin);
        m_session->add_extension(&lt::create_smart_ban_plugin);
    }

    m_session->set_alert_notify(
        [this]()
        {
            boost::asio::post(m_io, [this] { ReadAlerts(); });
        });

    if (options.timer_dht_stats > 0)
        m_timers.emplace_back(m_io, options.timer_dht_stats, [&]() { m_session->post_dht_stats(); });

    if (options.timer_session_stats > 0)
        m_timers.emplace_back(m_io, options.timer_session_stats, [&]() { m_session->post_session_stats(); });

    if (options.timer_torrent_updates > 0)
        m_timers.emplace_back(m_io, options.timer_torrent_updates, [&]() { m_session->post_torrent_updates(); });
}

Session::~Session()
{
    BOOST_LOG_TRIVIAL(info) << "Shutting down session";

    m_session->set_alert_notify([]{});
    m_timers.clear();

    WriteSessionParams(
            m_session_params_file,
            m_session->session_state());

    m_session->pause();

    int chunk_size = 1000;
    int chunks = static_cast<int>(m_torrents.size() / chunk_size) + 1;

    BOOST_LOG_TRIVIAL(info) << "Saving resume data in " << chunks << " chunk(s) - total torrents: " << m_torrents.size();

    auto current = m_torrents.begin();

    for (int i = 0; i < chunks; i++)
    {
        int chunk_items = std::min(
            chunk_size,
            static_cast<int>(std::distance(current, m_torrents.end())));

        int outstanding = 0;

        for (int j = 0; j < chunk_items; j++)
        {
            auto const& th = current->second;
            auto const& ts = th.status();

            if (!th.is_valid()
                || !ts.has_metadata
                || !ts.need_save_resume)
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

        BOOST_LOG_TRIVIAL(info) << "Chunk " << i + 1 << " - Saving state for " << outstanding << " torrent(s) (out of " << chunk_items << ")";

        while (outstanding > 0)
        {
            lt::alert const* tmp = m_session->wait_for_alert(lt::seconds(10));
            if (tmp == nullptr) { continue; }

            std::vector<lt::alert*> alerts;
            m_session->pop_alerts(&alerts);

            for (lt::alert* a : alerts)
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

                auto* rd = lt::alert_cast<lt::save_resume_data_alert>(a);
                if (!rd) { continue; }

                outstanding--;

                AddTorrentParams::Update(m_db, rd->handle.info_hashes(), AddTorrentParams{
                    .client_data    = rd->handle.userdata().get<TorrentClientData>(),
                    .name           = rd->params.name,
                    .params         = rd->params,
                    .queue_position = static_cast<int>(rd->handle.status().queue_position),
                    .save_path      = rd->params.save_path
                });
            }
        }
    }

    BOOST_LOG_TRIVIAL(info) << "All state saved";
}

void Session::Load()
{
    int count = AddTorrentParams::Count(m_db);
    int current = 0;

    BOOST_LOG_TRIVIAL(info) << "Loading " << count << " torrent(s) from storage";

    AddTorrentParams::ForEach(
        m_db,
        [&](lt::add_torrent_params& params)
        {
            current++;

            params.userdata.get<TorrentClientData>()->ignore_alert = true;

            lt::torrent_handle th = m_session->add_torrent(params);
            m_torrents.insert({ th.info_hashes(), th });

            if (current % 1000 == 0 && current != count)
            {
                BOOST_LOG_TRIVIAL(info) << current << " torrents (of " << count << ") added";
            }
        });

    if (count > 0)
    {
        BOOST_LOG_TRIVIAL(info) << "Added " << current << " (of " << count << ") torrent(s) to session";
    }
}

lt::info_hash_t Session::AddTorrent(lt::add_torrent_params const& p)
{
    m_session->async_add_torrent(p);

    if (p.ti)
    {
        return p.ti->info_hashes();
    }

    return p.info_hashes;
}

void Session::ApplySettings(const libtorrent::settings_pack& settings)
{
    BOOST_LOG_TRIVIAL(debug) << "Applying session settings";
    m_session->apply_settings(settings);
}

void Session::Pause()
{
    m_session->pause();
}

void Session::Recheck(const lt::info_hash_t &hash)
{
    const auto& handle = m_torrents.at(hash);

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
            if (!m_torrents.contains(hash))
            {
                return;
            }

            // TODO: Unsure about the order here. If there are reports that force-checking a torrent
            //       leads to any issues with resume/pause, the order of these statements might matter.

            if (was_auto_managed)
            {
                m_torrents.at(hash).set_flags(lt::torrent_flags::auto_managed);
            }

            if (was_paused)
            {
                m_torrents.at(hash).pause();
            }
        });

    handle.force_recheck();
}

void Session::Remove(const lt::info_hash_t& hash, bool remove_data)
{
    lt::torrent_handle th = m_torrents.at(hash);

    m_session->remove_torrent(th, remove_data ? lt::session::delete_files : lt::remove_flags_t{});
}

void Session::Resume()
{
    m_session->resume();
}

lt::settings_pack Session::Settings()
{
    return m_session->get_settings();
}

const std::map<lt::info_hash_t, lt::torrent_handle>& Session::Torrents()
{
    return m_torrents;
}

void Session::ReadAlerts()
{
    std::vector<lt::alert*> alerts;
    m_session->pop_alerts(&alerts);

    for (auto const alert : alerts)
    {
        BOOST_LOG_TRIVIAL(trace) << "Session alert: " << alert->message();

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

            m_torrents.insert({ ata->handle.info_hashes(), ata->handle });

            const auto status = ata->handle.status();

            AddTorrentParams::Insert(m_db, ata->handle.info_hashes(), AddTorrentParams{
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

            boost::asio::post(m_io, [this, status](){ m_torrentAdded(status); });

            break;
        }
        case lt::dht_stats_alert::alert_type:
        {
            auto dsa = lt::alert_cast<lt::dht_stats_alert>(alert);
            // TODO: emit signal
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

            AddTorrentParams::Update(m_db, status.info_hashes, AddTorrentParams{
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

            std::map<std::string, int64_t> metrics;

            for (auto const& stats : m_stats)
            {
                metrics.insert({ stats.name, counters[stats.value_index] });
            }

            boost::asio::post(m_io, [this, metrics](){ m_sessionStats(metrics); });

            break;
        }
        case lt::state_update_alert::alert_type:
        {
            auto sua = lt::alert_cast<lt::state_update_alert>(alert);

            boost::asio::post(m_io, [this, status = sua->status](){ m_stateUpdate(status); });

            break;
        }
        case lt::storage_moved_alert::alert_type:
        {
            auto const sma = lt::alert_cast<lt::storage_moved_alert>(alert);

            BOOST_LOG_TRIVIAL(info) << "Torrent " << sma->torrent_name() << " moved to " << sma->storage_path();

            if (sma->handle.need_save_resume_data())
            {
                sma->handle.save_resume_data(lt::torrent_handle::flush_disk_cache
                                             | lt::torrent_handle::save_info_dict
                                             | lt::torrent_handle::only_if_modified);
            }

            boost::asio::post(m_io, [this, th = sma->handle](){ m_storageMoved(th); });

            break;
        }
        case lt::torrent_checked_alert::alert_type:
        {
            const auto tca = lt::alert_cast<lt::torrent_checked_alert>(alert);
            BOOST_LOG_TRIVIAL(info) << "Torrent " << tca->torrent_name() << " finished checking";

            if (m_oneshot_torrent_callbacks.contains({ alert->type(), tca->handle.info_hashes()}))
            {
                for (auto && cb : m_oneshot_torrent_callbacks.at({ alert->type(), tca->handle.info_hashes() }))
                {
                    cb();
                }

                m_oneshot_torrent_callbacks.erase({ alert->type(), tca->handle.info_hashes() });
            }

            break;
        }
        case lt::torrent_finished_alert::alert_type:
        {
            const auto tfa          = lt::alert_cast<lt::torrent_finished_alert>(alert);
            const auto& status      = tfa->handle.status();
            const auto& client_data = tfa->handle.userdata().get<TorrentClientData>();

            if (status.total_download > 0)
            {
                // Only emit this event if we have downloaded any data this session
                BOOST_LOG_TRIVIAL(info) << "Torrent " << status.name << " finished";

                boost::asio::post(m_io, [this, status](){ m_torrentFinished(status); });
            }

            if (status.need_save_resume)
            {
                status.handle.save_resume_data(lt::torrent_handle::flush_disk_cache
                                               | lt::torrent_handle::save_info_dict
                                               | lt::torrent_handle::only_if_modified);
            }

            break;
        }
        case lt::torrent_paused_alert::alert_type:
        {
            auto tpa = lt::alert_cast<lt::torrent_paused_alert>(alert);

            BOOST_LOG_TRIVIAL(debug) << "Torrent " << tpa->torrent_name() << " paused";

            boost::asio::post(m_io, [this, th = tpa->handle](){ m_torrentPaused(th); });

            break;
        }
        case lt::torrent_removed_alert::alert_type:
        {
            auto tra = lt::alert_cast<lt::torrent_removed_alert>(alert);

            AddTorrentParams::Remove(m_db, tra->info_hashes);

            m_torrents.erase(tra->info_hashes);

            BOOST_LOG_TRIVIAL(info) << "Torrent " << tra->torrent_name() << " removed";

            boost::asio::post(m_io, [this, hash = tra->info_hashes](){ m_torrentRemoved(hash); });

            break;
        }
        case lt::torrent_resumed_alert::alert_type:
        {
            auto tra = lt::alert_cast<lt::torrent_resumed_alert>(alert);
            auto const& status = tra->handle.status();

            BOOST_LOG_TRIVIAL(debug) << "Torrent " << status.name << " resumed";

            boost::asio::post(m_io, [this, status](){ m_torrentResumed(status); });

            break;
        }
        case lt::tracker_error_alert::alert_type:
        {
            const auto tea = lt::alert_cast<lt::tracker_error_alert>(alert);
            m_torrentTrackerError(tea);
            break;
        }
        case lt::tracker_reply_alert::alert_type:
        {
            const auto tra = lt::alert_cast<lt::tracker_reply_alert>(alert);
            m_torrentTrackerReply(tra->handle);
            break;
        }
        }
    }
}
