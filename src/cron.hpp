#pragma once
#include <chrono>
#include <ctime>
#include <functional>
#include <memory>
#include <string>

#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <croncpp.hpp>

class CronSchedule : public std::enable_shared_from_this<CronSchedule>
{
public:
    static std::shared_ptr<CronSchedule> Create(
        boost::asio::io_context& io,
        const std::string& expression,
        std::function<void()> on_tick)
    {
        cron::cronexpr expr = cron::make_cron(expression);

        auto s = std::shared_ptr<CronSchedule>(
            new CronSchedule(io, std::move(expr), std::move(on_tick)));
        s->Next();

        return s;
    }

    ~CronSchedule() { Cancel(); }

    void Cancel()
    {
        m_cancelled = true;
        m_on_tick   = nullptr;
        m_timer.cancel();
    }

private:
    CronSchedule(boost::asio::io_context& io, cron::cronexpr expr, std::function<void()> on_tick)
        : m_expr(std::move(expr))
        , m_on_tick(std::move(on_tick))
        , m_cancelled(false)
        , m_timer(io) {}

    void Next()
    {
        const std::time_t now  = std::time(nullptr);
        const std::time_t next = cron::cron_next(m_expr, now);

        m_timer.expires_after(std::chrono::seconds(next - now));
        m_timer.async_wait(
            [self = shared_from_this()](const boost::system::error_code& ec)
            { self->OnTimerExpired(ec); });
    }

    void OnTimerExpired(const boost::system::error_code& ec)
    {
        if (ec) return;                       // aborted (cancel) or error -> stop
        if (m_cancelled || !m_on_tick) return;

        m_on_tick();                          // State decides how to run the Lua fn

        if (!m_cancelled) Next();             // on_tick may have cancelled us
    }

    cron::cronexpr            m_expr;
    std::function<void()>     m_on_tick;
    bool                      m_cancelled;
    boost::asio::steady_timer m_timer;
};
