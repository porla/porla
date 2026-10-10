#include "../session.hpp"

#include <boost/log/trivial.hpp>

#include <libtorrent/alert_types.hpp>
#include <libtorrent/session_stats.hpp>

#include "../jobs/reconciletorrents.hpp"
#include "../publish.hpp"
#include "../scheduler.hpp"
#include "../sessionevent.hpp"

#include "../../events.hpp"

using porla::Session;

namespace
{
    const std::vector<lt::stats_metric> kSessionMetrics = lt::session_stats_metrics();
}

void Session::OnAlertsDroppedAlert(const lt::alerts_dropped_alert* alert)
{
    std::string types;

    for (int i = 0; i < lt::num_alert_types; i++)
    {
        if (alert->dropped_alerts.test(i))
        {
            types += (types.empty() ? "" : ",") + std::string(lt::alert_name(i));
        }
    }

    BOOST_LOG_TRIVIAL(warning)
        << Log() << "The libtorrent alert queue is full and dropped "
        << types << " alerts. Consider raising alert_queue_size.";

    if (alert->dropped_alerts.test(lt::add_torrent_alert::alert_type)
        || alert->dropped_alerts.test(lt::torrent_removed_alert::alert_type))
    {
        m_jobs->Trigger<Jobs::ReconcileTorrents>();
    }
}

void Session::OnListenFailedAlert(const lt::listen_failed_alert* alert)
{
    BOOST_LOG_TRIVIAL(warning) << Log() << alert->message();
}

void Session::OnListenSucceededAlert(const lt::listen_succeeded_alert* alert)
{
    BOOST_LOG_TRIVIAL(info) << Log() << alert->message();
}

void Session::OnSessionStatsAlert(const lt::session_stats_alert* alert)
{
    if (!m_options.events.HasSubscribers("session.stats"))
    {
        return;
    }

    const auto counters = alert->counters();

    nlohmann::json stats = nlohmann::json::object();

    for (const auto& m : kSessionMetrics)
    {
        stats[m.name] = counters[m.value_index];
    }

    Publish(SessionEvent("session.stats", {
        { "stats", std::move(stats) }
    }));
}

void Session::OnStateUpdateAlert(const lt::state_update_alert* alert)
{
    for (const auto& status : alert->status)
    {
        const auto it = m_torrents.find(status.info_hashes);

        // recieved alert for missing torrent or torrent not Current - we
        // could have missed an alert
        if (it == m_torrents.end() || it->second.state != Torrent::State::Current)
        {
            BOOST_LOG_TRIVIAL(debug)
                << Log(status.info_hashes) << "Received state update for non-tracked torrent";

            m_jobs->Trigger<Jobs::ReconcileTorrents>();

            continue;
        }

        it->second.status = status;
    }
}
