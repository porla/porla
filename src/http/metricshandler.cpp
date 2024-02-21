#include "metricshandler.hpp"

#include <boost/signals2.hpp>
#include <libtorrent/session_stats.hpp>

#include "../sessions.hpp"

using porla::Http::MetricsHandler;

static std::map<std::string, std::string> MetricHelp =
{
    {"", ""}
};

class MetricsHandler::State
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

    std::map<lt::sha1_hash, std::tuple<std::uint64_t, std::string, libtorrent::torrent_status>>& TorrentStatuses()
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
            m_torrent_statuses.insert_or_assign(status.info_hashes.v1, std::make_tuple(ms,session, status));
        }
    }

    boost::signals2::connection m_stats_connection;
    boost::signals2::connection m_state_update_connection;
    std::map<std::string, std::pair<std::uint64_t, lt::span<const int64_t>>> m_session_counters;
    std::map<lt::sha1_hash, std::tuple<std::uint64_t, std::string, libtorrent::torrent_status>> m_torrent_statuses;
    std::vector<lt::stats_metric> m_stats;
};

MetricsHandler::MetricsHandler(porla::Sessions& sessions)
    : m_state(std::make_shared<State>(sessions))
{
}

MetricsHandler::~MetricsHandler() = default;

void MetricsHandler::operator()(uWS::HttpResponse<false>* res, [[maybe_unused]] uWS::HttpRequest* req)
{
    std::stringstream out;
    this->PopulateWithGlobalMetrics(out);
    this->PopulateWithPerTorrentMetrics(out);

    res->writeStatus("200 OK")->end(out.str());
}

void MetricsHandler::PopulateWithGlobalMetrics(std::stringstream& out) {

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

void MetricsHandler::PopulateWithPerTorrentMetrics(std::stringstream &out) {
    for (const auto &[info_hash, tuple]: m_state->TorrentStatuses()) {
        const auto [ms, session, status] = tuple;
        auto makeCounter = [&out, &session, &ms](
                const char *name,
                const std::int64_t value
        ) {
            out << "# TYPE porla_torrent_status_" << name << " counter\n";
            out << "porla_torrent_status_" << name << "{session=\"" << session << "\", name=\"" << name << "\"} " << value
                << " " << ms << "\n\n";
        };
        makeCounter("total_download", status.total_download);
        makeCounter("total_upload", status.total_upload);
        makeCounter("total_payload_download", status.total_payload_download);
        makeCounter("total_payload_upload", status.total_payload_upload);
        makeCounter("total_failed_bytes", status.total_failed_bytes);
        makeCounter("total_redundant_bytes", status.total_redundant_bytes);
        makeCounter("total_done", status.total_done);
        makeCounter("total", status.total);
        makeCounter("total_wanted_done", status.total_wanted_done);
        makeCounter("total_wanted", status.total_wanted);
        makeCounter("all_time_upload", status.all_time_upload);
        makeCounter("all_time_download", status.all_time_download);
        makeCounter("added_time", status.added_time);
        makeCounter("completed_time", status.completed_time);
        makeCounter("last_seen_complete", status.last_seen_complete);
        makeCounter("progress_ppm",  status.progress_ppm);
        makeCounter("queue_position", status.queue_position.operator int());
        makeCounter("download_rate", status.download_rate);
        makeCounter("upload_rate", status.upload_rate);
        makeCounter("download_payload_rate", status.download_payload_rate);
        makeCounter("upload_payload_rate", status.upload_payload_rate);
        makeCounter("num_seeds", status.num_seeds);
        makeCounter("num_peers", status.num_peers);
        makeCounter("num_complete", status.num_complete);
        makeCounter("num_incomplete", status.num_incomplete);
        makeCounter("list_seeds", status.list_seeds);
        makeCounter("list_peers", status.list_peers);
        makeCounter("connect_candidates", status.connect_candidates);
        makeCounter("num_pieces", status.num_pieces);
        makeCounter("distributed_full_copies", status.distributed_full_copies);
        makeCounter("distributed_fraction", status.distributed_fraction);
        makeCounter("block_size", status.block_size);
        makeCounter("num_uploads", status.num_uploads);
        makeCounter("num_connections", status.num_connections);
        makeCounter("uploads_limit", status.uploads_limit);
        makeCounter("connections_limit", status.connections_limit);
        makeCounter("up_bandwidth_queue", status.up_bandwidth_queue);
        makeCounter("down_bandwidth_queue", status.down_bandwidth_queue);
        makeCounter("seed_rank", status.seed_rank);
        makeCounter("state", status.state);
        makeCounter("active_duration", status.active_duration.count());
        makeCounter("finished_duration", status.finished_duration.count());
        makeCounter("seeding_duration", status.seeding_duration.count());
    }
}
