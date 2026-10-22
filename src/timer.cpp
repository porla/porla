#include "timer.hpp"

#include <ctime>

#include <boost/log/trivial.hpp>
#include <croncpp.hpp>

using porla::Timer;

namespace
{
    std::optional<std::chrono::milliseconds> CronDelay(const cron::cronexpr& expr)
    {
        const std::time_t now  = std::time(nullptr);
        const std::time_t next = cron::cron_next(expr, now);

        if (next == static_cast<std::time_t>(-1) || next <= now)
        {
            BOOST_LOG_TRIVIAL(error) << "Cron expression has no future execution - stopping";
            return std::nullopt;
        }

        return std::chrono::seconds(next - now);
    }
}
std::shared_ptr<Timer> Timer::Interval(boost::asio::io_context& io, std::chrono::milliseconds interval, std::function<void()> cb)
{
    return Start(io, [interval]() { return std::optional(interval); }, std::move(cb));
}

std::shared_ptr<Timer> Timer::Cron(boost::asio::io_context& io, const std::string& expression, std::function<void()> cb)
{
    return Start(io, [expr = cron::make_cron(expression)]() { return CronDelay(expr); }, std::move(cb));
}

std::shared_ptr<Timer> Timer::Start(boost::asio::io_context& io, NextDelay next, std::function<void()> cb)
{
    auto t = std::shared_ptr<Timer>(new Timer(io, std::move(next), std::move(cb)));
    t->Arm();
    return t;
}

Timer::Timer(boost::asio::io_context& io, NextDelay next, std::function<void()> cb)
    : m_cancelled(false)
    , m_timer(io)
    , m_next(std::move(next))
    , m_callback(std::move(cb))
{
}

Timer::~Timer()
{
    m_cancelled = true;
    m_timer.cancel();
}

void Timer::Cancel()
{
    m_cancelled = true;
    m_timer.cancel();
}

void Timer::Arm()
{
    const auto delay = m_next();

    if (!delay.has_value())
    {
        return;
    }

    m_timer.expires_after(std::chrono::milliseconds(*delay));
    m_timer.async_wait(
        [weak = weak_from_this()](boost::system::error_code ec)
        {
            if (auto self = weak.lock()) { self->OnExpired(ec); }
        });
}

void Timer::OnExpired(boost::system::error_code ec)
{
    if (m_cancelled || ec == boost::asio::error::operation_aborted)
    {
        return;
    }
    else if (ec)
    {
        BOOST_LOG_TRIVIAL(error) << "Error in timer: " << ec.message();
        return;
    }

    Arm();

    try
    {
        m_callback();
    }
    catch (const std::exception& ex)
    {
        BOOST_LOG_TRIVIAL(error) << "Unhandled exception in timer callback: " << ex.what();
    }
    catch (...)
    {
        BOOST_LOG_TRIVIAL(error) << "Unknown exception in timer callback";
    }
}
