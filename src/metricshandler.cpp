#include "metricshandler.hpp"

#include "session.hpp"

using porla::MetricsHandler;

MetricsHandler::MetricsHandler(porla::ISession &session)
    : m_session(session)
{
    m_sessionStatsConnection = m_session.OnSessionStats([this](auto s) { OnSessionStats(s); });
}

MetricsHandler::~MetricsHandler()
{
    m_sessionStatsConnection.disconnect();
}

void MetricsHandler::operator()(const std::shared_ptr<porla::HttpContext> &ctx)
{
    std::stringstream out;

    for (auto const& [key,val] : m_stats)
    {
        std::string key_replaced = key;
        std::replace(key_replaced.begin(), key_replaced.end(), '.', '_');

        out << "libtorrent_" << key_replaced << " " << val << "\n";
    }

    ctx->Write(out.str());
}

void MetricsHandler::OnSessionStats(const std::map<std::string, int64_t>& stats)
{
    m_stats = stats;
}
