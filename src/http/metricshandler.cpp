#include "metricshandler.hpp"

#include "../session.hpp"

using porla::Http::MetricsHandler;

class MetricsHandler::State
{
public:
    explicit State(ISession& session)
    {
        m_stats_connection = session.OnSessionStats([this](auto s) { OnSessionStats(s); });
    }

    ~State()
    {
        m_stats_connection.disconnect();
    }

    std::map<std::string, int64_t>& Stats()
    {
        return m_stats;
    }

private:
    void OnSessionStats(const std::map<std::string, int64_t>& stats)
    {
        m_stats = stats;
    }

    boost::signals2::connection m_stats_connection;
    std::map<std::string, int64_t> m_stats;
};

MetricsHandler::MetricsHandler(porla::ISession& session)
    : m_state(std::make_shared<State>(session))
{
}

MetricsHandler::~MetricsHandler() = default;

void MetricsHandler::operator()(uWS::HttpResponse<false>* res, uWS::HttpRequest* req)
{
    std::stringstream out;

    for (auto const& [key,val] : m_state->Stats())
    {
        std::string key_replaced = key;
        std::replace(key_replaced.begin(), key_replaced.end(), '.', '_');

        out << "libtorrent_" << key_replaced << " " << val << "\n";
    }

    res->writeStatus("200 OK")->end(out.str());
}
