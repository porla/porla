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
    }

    ~State()
    {
        m_stats_connection.disconnect();
    }

    std::map<std::string, std::pair<std::uint64_t, lt::span<const int64_t>>>& SessionCounters()
    {
        return m_session_counters;
    }

    std::vector<lt::stats_metric>& StatsMetrics()
    {
        return m_stats;
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

    boost::signals2::connection m_stats_connection;
    std::map<std::string, std::pair<std::uint64_t, lt::span<const int64_t>>> m_session_counters;
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

    res->writeStatus("200 OK")->end(out.str());
}
