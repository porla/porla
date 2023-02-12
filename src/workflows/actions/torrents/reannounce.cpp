#include "reannounce.hpp"

#include <boost/log/trivial.hpp>
#include <libtorrent/alert_types.hpp>

#include "../../../json/lttorrentstatus.hpp"
#include "../../../session.hpp"

using porla::Workflows::Actions::Torrents::Reannounce;

struct Reannounce::TorrentReannounceState
{
    std::shared_ptr<ActionCallback> callback;
    int                             current_tries{0};
    int                             max_tries{24};
    int                             timeout{5};
};

Reannounce::Reannounce(porla::ISession& session)
    : m_session(session)
{
    m_torrent_tracker_error_connection = m_session.OnTorrentTrackerError([this](auto && th) { OnTorrentTrackerError(th); });
    m_torrent_tracker_reply_connection = m_session.OnTorrentTrackerReply([this](auto && th) { OnTorrentTrackerReply(th); });
}

Reannounce::~Reannounce()
{
    m_torrent_tracker_error_connection.disconnect();
    m_torrent_tracker_reply_connection.disconnect();
}

void Reannounce::Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback)
{
    const auto& torrent = params.Render("torrent", true);

    if (torrent == nullptr)
    {
        // WARN
        return;
    }

    const lt::torrent_status& ts = torrent;
    const auto& th = m_session.Torrents().find(ts.info_hashes);

    if (th == m_session.Torrents().end())
    {
        return;
    }

    BOOST_LOG_TRIVIAL(info) << "Reannouncing torrent " << th->second.status().name;

    // Force an immediate reannounce and ignore the min interval.
    th->second.force_reannounce(0, -1, lt::torrent_handle::ignore_min_interval);

    auto state = std::make_unique<TorrentReannounceState>();
    state->callback      = callback;
    state->current_tries = 0;
    state->max_tries     = 24;
    state->timeout       = 5;

    if (params.Input().contains("max_tries"))
    {
        state->max_tries = params.Input()["max_tries"].get<int>();
    }

    if (params.Input().contains("timeout"))
    {
        state->timeout = params.Input()["timeout"].get<int>();
    }

    m_states.insert({ ts.info_hashes, std::move(state) });
}

void Reannounce::OnTorrentTrackerError(const libtorrent::tracker_error_alert* al)
{
    auto ctx = m_states.find(al->handle.info_hashes());
    if (ctx == m_states.end()) return;

    ctx->second->current_tries++;

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
                BOOST_LOG_TRIVIAL(info) << "Reannouncing torrent " << al->torrent_name();

                al->handle.force_reannounce(ctx->second->timeout, -1, lt::torrent_handle::ignore_min_interval);
                return;
            }
        }
    }
}

void Reannounce::OnTorrentTrackerReply(const libtorrent::torrent_handle& th)
{
    auto ctx = m_states.find(th.info_hashes());
    if (ctx == m_states.end()) return;

    BOOST_LOG_TRIVIAL(info) << "Reannouncing done";

    ctx->second->callback->Complete(true);

    m_states.erase(ctx);
}

