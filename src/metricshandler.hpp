#pragma once

#include <boost/signals2.hpp>

#include "httpcontext.hpp"

namespace porla
{
    class ISession;

    class MetricsHandler
    {
    public:
        explicit MetricsHandler(ISession& session);
        explicit MetricsHandler(const MetricsHandler&) = delete;
        explicit MetricsHandler(const MetricsHandler&&) = delete;

        ~MetricsHandler();

        void operator()(const std::shared_ptr<porla::HttpContext>& ctx);

    private:
        void OnSessionStats(const std::map<std::string, int64_t>& stats);

        ISession& m_session;
        boost::signals2::connection m_sessionStatsConnection;
        std::map<std::string, int64_t> m_stats;
    };
}
