#include "../globals.hpp"

#include <chrono>

#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <croncpp.hpp>

#include "../registry.hpp"

using porla::Lua::Globals::Cron;

class CronSchedule : public std::enable_shared_from_this<CronSchedule>
{
public:
    static std::shared_ptr<CronSchedule> Create(
        boost::asio::io_context& io,
        const std::string& expression,
        sol::function callback)
    {
        cron::cronexpr expr = cron::make_cron(expression);

        auto schedule = std::shared_ptr<CronSchedule>(
            new CronSchedule(io, std::move(expr), std::move(callback)));

        schedule->Next();

        return schedule;
    }

    ~CronSchedule()
    {
        BOOST_LOG_TRIVIAL(debug) << "Destroying cron schedule";
        Cancel();
    }

    void Cancel()
    {
        m_should_cancel = true;
        m_timer.cancel();
    }

private:
    CronSchedule(boost::asio::io_context& io, cron::cronexpr expr, sol::function callback)
        : m_expr(std::move(expr))
        , m_callback(std::move(callback))
        , m_should_cancel(false)
        , m_timer(io)
    {
    }

    void Next()
    {
        const std::time_t now  = std::time(nullptr);
        const std::time_t next = cron::cron_next(m_expr, now);
        const auto seconds     = next - now;

        BOOST_LOG_TRIVIAL(debug) << "Next cron invocation in " << seconds << " seconds";

        m_timer.expires_after(std::chrono::seconds(seconds));

        // Capture a shared_ptr so the object outlives any in-flight handler,
        // even if Lua GCs its reference while the timer is pending.
        m_timer.async_wait(
            [self = shared_from_this()](const boost::system::error_code& ec)
            {
                self->OnTimerExpired(ec);
            });
    }

    void OnTimerExpired(const boost::system::error_code& ec)
    {
        if (ec)
        {
            if (ec != boost::asio::error::operation_aborted)
            {
                BOOST_LOG_TRIVIAL(error) << "Timer error: " << ec.message();
            }

            return;
        }

        try
        {
            m_callback();
        }
        catch (const sol::error& err)
        {
            BOOST_LOG_TRIVIAL(error) << "Error when invoking cron callback: " << err.what();
        }

        if (!m_should_cancel)
        {
            Next();
        }
    }

    cron::cronexpr        m_expr;
    sol::function         m_callback;
    bool                  m_should_cancel;
    boost::asio::steady_timer m_timer;
};

sol::object Cron::Build(sol::state& lua)
{
    lua.new_usertype<CronSchedule>(
        "CronSchedule",
        sol::no_constructor,
        "cancel", &CronSchedule::Cancel);

    return sol::make_object(lua, [](sol::this_state L, const std::string& expression, sol::function callback)
    {
        sol::state_view lua(L);
        auto io = lua.registry()["io"].get<porla::Lua::Registry::BoostIoContext>().io;
        return CronSchedule::Create(*io, expression, std::move(callback));
    });
}
