#include "torrentreannounce.hpp"

#include <boost/log/trivial.hpp>
#include <libtorrent/alert_types.hpp>

#include "../../../session.hpp"

using porla::Lua::Workflows::ActionCallback;
using porla::Lua::Workflows::Actions::TorrentReannounce;
using porla::Lua::Workflows::Actions::TorrentReannounceOptions;

struct WorkItem
{
    std::shared_ptr<ActionCallback> callback;
    int                             current_tries{0};
    lt::info_hash_t                 info_hash;
};

class TorrentReannounce::State
{
public:
    explicit State(const TorrentReannounceOptions& opts)
        : m_opts(opts)
    {
        m_on_tracker_error = opts.session.OnTorrentTrackerError([this](auto && th) { OnTorrentTrackerError(th); });
        m_on_tracker_reply = opts.session.OnTorrentTrackerReply([this](auto && th) { OnTorrentTrackerReply(th); });
    }

    ~State()
    {
        m_on_tracker_error.disconnect();
        m_on_tracker_reply.disconnect();
    }

    void Announce(const lt::torrent_handle& th, std::shared_ptr<ActionCallback> callback)
    {
        if (m_work_item != nullptr)
        {
            BOOST_LOG_TRIVIAL(error) << "Reannounce already running";
            return;
        }

        BOOST_LOG_TRIVIAL(info) << "Reannouncing torrent " << th.status().name;

        // Force an immediate reannounce and ignore the min interval.
        th.force_reannounce(0, -1, lt::torrent_handle::ignore_min_interval);

        m_work_item                = std::make_unique<WorkItem>();
        m_work_item->callback      = std::move(callback);
        m_work_item->current_tries = 0;
        m_work_item->info_hash     = th.info_hashes();
    }

private:
    void OnTorrentTrackerError(const libtorrent::tracker_error_alert* al)
    {
        if (m_work_item == nullptr)                             return;
        if (al->handle.info_hashes() != m_work_item->info_hash) return;

        if (al->error && al->error.value() == lt::errors::tracker_failure)
        {
            const std::vector<std::string> match_failures =
            {
                "not exist",
                "not found",
                "not registered",
                "unregistered"
            };

            const std::string err = al->error_message();

            for (const auto& failure_message : match_failures)
            {
                if (err.find(failure_message) != std::string::npos)
                {
                    m_work_item->current_tries++;

                    if (m_work_item->current_tries >= m_opts.max_tries)
                    {
                        BOOST_LOG_TRIVIAL(warning) << "Max reannounce attempts reached for " << al->torrent_name();
                        m_work_item = nullptr;
                        return;
                    }

                    BOOST_LOG_TRIVIAL(info)
                        << "Reannouncing torrent " << al->torrent_name()
                        << " - attempt " << m_work_item->current_tries << " of " << m_opts.max_tries;

                    al->handle.force_reannounce(m_opts.timeout, -1, lt::torrent_handle::ignore_min_interval);
                }
            }
        }
    }

    void OnTorrentTrackerReply(const libtorrent::torrent_handle& th)
    {
        if (m_work_item == nullptr)                     return;
        if (th.info_hashes() != m_work_item->info_hash) return;

        m_work_item->callback->Complete();
        m_work_item = nullptr;
    }

    boost::signals2::connection m_on_tracker_error;
    boost::signals2::connection m_on_tracker_reply;
    TorrentReannounceOptions    m_opts;
    std::unique_ptr<WorkItem>   m_work_item;
};

TorrentReannounce::TorrentReannounce(const TorrentReannounceOptions& opts)
{
    m_state = std::make_unique<State>(opts);
}

TorrentReannounce::~TorrentReannounce() = default;

void TorrentReannounce::Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback)
{
    const lt::torrent_handle& th = params.context["lt:torrent_handle"];
    m_state->Announce(th, callback);
}
