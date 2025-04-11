#include "metricshandler.hpp"

#include <boost/signals2.hpp>
#include <libtorrent/session_stats.hpp>

#include "../sessions.hpp"

using porla::Http::MetricsHandler;

static std::map<std::string, std::string> MetricHelp =
{
    {"", ""}
};

template <bool SSL> class MetricsHandler<SSL>::State
{
public:
    explicit State(Sessions& sessions)
        : m_stats(lt::session_stats_metrics())
    {
        m_stats_connection = sessions.OnSessionStats([this](auto && p1, auto && p2) { OnSessionStats(p1, p2); });
        m_state_update_connection = sessions.OnStateUpdate([this](auto && p1, auto && p2) { OnStateUpdate(p1, p2); });
    }

    ~State()
    {
        m_stats_connection.disconnect();
        m_state_update_connection.disconnect();
    }

    std::map<std::string, std::pair<std::uint64_t, lt::span<const int64_t>>>& SessionCounters()
    {
        return m_session_counters;
    }

    std::vector<lt::stats_metric>& StatsMetrics()
    {
        return m_stats;
    }

    std::map<lt::info_hash_t, std::tuple<std::uint64_t, std::string, libtorrent::torrent_status>>& TorrentStatuses()
    {
        return m_torrent_statuses;
    }

private:
    void OnSessionStats(const std::string& session, const lt::span<const int64_t>& stats)
    {
        if (m_session_counters.find(session) == m_session_counters.end())
        {
            m_session_counters.insert({ session, {} });
        }

        uint64_t ms = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

        m_session_counters.at(session) = { ms, stats};
    }

    void OnStateUpdate(const std::string& session, const std::vector<libtorrent::torrent_status>& torrents)
    {
        for (const auto& status : torrents)
        {
            uint64_t ms = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            m_torrent_statuses.insert_or_assign(status.info_hashes, std::make_tuple(ms,session, status));
        }
    }

    boost::signals2::connection m_stats_connection;
    boost::signals2::connection m_state_update_connection;
    std::map<std::string, std::pair<std::uint64_t, lt::span<const int64_t>>> m_session_counters;
    std::map<lt::info_hash_t, std::tuple<std::uint64_t, std::string, libtorrent::torrent_status>> m_torrent_statuses;
    std::vector<lt::stats_metric> m_stats;
};

template <bool SSL> MetricsHandler<SSL>::MetricsHandler(porla::Sessions& sessions)
    : m_state(std::make_shared<State>(sessions))
{
}

template <bool SSL> MetricsHandler<SSL>::~MetricsHandler() = default;

template <bool SSL> void MetricsHandler<SSL>::operator()(uWS::HttpResponse<SSL>* res, [[maybe_unused]] uWS::HttpRequest* req)
{
    std::stringstream out;
    this->PopulateWithGlobalMetrics(out);
    this->PopulateWithPerTorrentMetrics(out);

    res->writeStatus("200 OK")->end(out.str());
}

template <bool SSL> void MetricsHandler<SSL>::PopulateWithGlobalMetrics(std::stringstream& out) {

    for (const auto& metric : m_state->StatsMetrics())
    {
        std::string key_replaced = metric.name;
        std::replace(key_replaced.begin(), key_replaced.end(), '.', '_');

        // # HELP
        // # TYPE

        const auto help = MetricHelp.find(metric.name);

        if (help != MetricHelp.end())
        {
            out << "# HELP libtorrent_" << key_replaced << " " << help->second << "\n";
        }

        switch (metric.type)
        {
            case libtorrent::metric_type_t::counter:
                out << "# TYPE libtorrent_" << key_replaced << " counter\n";
                break;
            case libtorrent::metric_type_t::gauge:
                out << "# TYPE libtorrent_" << key_replaced << " gauge\n";
                break;
        }

        for (const auto& [ session, counters ] : m_state->SessionCounters())
        {
            if (counters.first == 0)
            {
                continue;
            }

            const auto counter_value = counters.second[metric.value_index];

            out << "libtorrent_" << key_replaced << "{session=\"" << session << "\"} " << counter_value << " " << counters.first << "\n";
        }

        out << "\n";
    }
}

template <bool SSL> void MetricsHandler<SSL>::PopulateWithPerTorrentMetrics(std::stringstream &out) {
    for (const auto &[info_hash, tuple]: m_state->TorrentStatuses()) {
        const auto [ms, session, status] = tuple;
        auto metric = [&out, &session, &ms, &status](
                const char *type,
                const char *name,
                const std::int64_t value
        ) {
            out << "# TYPE porla_torrent_status_" << name << " " << type << "\n";
            out << "porla_torrent_status_" << name << "{session=\"" << session << "\", name=\"" << status.name << "\"} " << value
                << " " << ms << "\n\n";
        };
        metric("counter", "total_download", status.total_download);
        metric("counter", "total_upload", status.total_upload);
        metric("counter", "total_payload_download", status.total_payload_download);
        metric("counter", "total_payload_upload", status.total_payload_upload);
        metric("counter", "total_failed_bytes", status.total_failed_bytes);
        metric("counter", "total_redundant_bytes", status.total_redundant_bytes);
        metric("counter", "total_done", status.total_done);
        metric("counter", "total", status.total);
        metric("counter", "all_time_upload", status.all_time_upload);
        metric("counter", "all_time_download", status.all_time_download);
        metric("gauge", "added_time", status.added_time);
        metric("gauge", "completed_time", status.completed_time);
        metric("counter", "progress_ppm",  status.progress_ppm);
        metric("gauge", "queue_position", status.queue_position.operator int());
        metric("gauge", "download_rate", status.download_rate);
        metric("gauge", "upload_rate", status.upload_rate);
        metric("gauge", "download_payload_rate", status.download_payload_rate);
        metric("gauge", "upload_payload_rate", status.upload_payload_rate);
        metric("gauge", "num_seeds", status.num_seeds);
        metric("gauge", "num_peers", status.num_peers);
        metric("gauge", "num_complete", status.num_complete);
        metric("gauge", "num_incomplete", status.num_incomplete);
        metric("gauge", "list_seeds", status.list_seeds);
        metric("gauge", "list_peers", status.list_peers);
        metric("gauge", "connect_candidates", status.connect_candidates);
        metric("gauge", "num_pieces", status.num_pieces);
        metric("gauge", "distributed_full_copies", status.distributed_full_copies);
        metric("gauge", "distributed_fraction", status.distributed_fraction);
        metric("gauge", "block_size", status.block_size);
        metric("gauge", "num_uploads", status.num_uploads);
        metric("gauge", "num_connections", status.num_connections);
        metric("gauge", "uploads_limit", status.uploads_limit);
        metric("gauge", "connections_limit", status.connections_limit);
        metric("gauge", "up_bandwidth_queue", status.up_bandwidth_queue);
        metric("gauge", "down_bandwidth_queue", status.down_bandwidth_queue);
        metric("gauge", "seed_rank", status.seed_rank);
        metric("gauge", "state", status.state);
        metric("counter", "active_duration", status.active_duration.count());
        metric("counter", "finished_duration", status.finished_duration.count());
        metric("counter", "seeding_duration", status.seeding_duration.count());
    }
}

namespace porla::Http {
    template class MetricsHandler<true>;
    template class MetricsHandler<false>;
}
