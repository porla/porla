#include "metricshandler.hpp"

#include <boost/signals2.hpp>

#include "../sessions.hpp"

using porla::Http::MetricsHandler;

class MetricsHandler::State
{
public:
    explicit State(Sessions& sessions)
    {
        m_stats_connection = sessions.OnSessionStats([this](auto && p1, auto && p2) { OnSessionStats(p1, p2); });
    }

    ~State()
    {
        m_stats_connection.disconnect();
    }

    std::map<std::string, std::map<std::string, int64_t>>& Stats()
    {
        return m_stats;
    }

private:
    void OnSessionStats(const std::string& session, const std::map<std::string, int64_t>& stats)
    {
        if (m_stats.find(session) == m_stats.end())
        {
            m_stats.insert({ session, {} });
        }

        m_stats.at(session) = stats;
    }

    boost::signals2::connection m_stats_connection;
    std::map<std::string, std::map<std::string, int64_t>> m_stats;
};

MetricsHandler::MetricsHandler(porla::Sessions& sessions)
    : m_state(std::make_shared<State>(sessions))
{
}

MetricsHandler::~MetricsHandler() = default;

void MetricsHandler::operator()(uWS::HttpResponse<false>* res, uWS::HttpRequest* req)
{
    std::stringstream out;

    for (const auto& [session, stats] : m_state->Stats())
    {
        for (const auto& [key, val] : stats)
        {
            std::string key_replaced = key;
            std::replace(key_replaced.begin(), key_replaced.end(), '.', '_');

            out << "libtorrent_" << key_replaced << "{session=\"" << session << "\"} " << val << "\n";
        }
    }

    res->writeStatus("200 OK")->end(out.str());
}
