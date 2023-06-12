#include "cron.hpp"

#include <boost/log/trivial.hpp>
#include <croncpp.hpp>

using porla::Lua::UserTypes::Cron;

void Cron::Register(sol::state& lua)
{
    sol::usertype<Cron> type = lua.new_usertype<Cron>(
        "cron",
        sol::no_constructor);
}

Cron::Cron(boost::asio::io_context &io, sol::table args)
    : m_io(io)
    , m_args(std::move(args))
    , m_timer(io)
{
    ScheduleNext();
}

Cron::~Cron() = default;

void Cron::OnTimerExpired(const boost::system::error_code &ec)
{
    if (m_args["callback"].is<sol::function>())
    {
        m_args["callback"]();
    }

    ScheduleNext();
}

void Cron::ScheduleNext()
{
    cron::cronexpr expression;

    try
    {
        expression = cron::make_cron(m_args["expression"].get<std::string>());
    }
    catch (const cron::bad_cronexpr& err)
    {
        BOOST_LOG_TRIVIAL(error) << "(cron) Failed to parse cron expression: " << err.what();
        return;
    }

    const std::time_t now = std::time(nullptr);
    const std::time_t next = cron::cron_next(expression, now);

    const auto seconds = next - now;

    BOOST_LOG_TRIVIAL(debug) << "(cron) Next invocation in " << seconds << " seconds";

    m_timer.expires_from_now(boost::posix_time::seconds(seconds));
    m_timer.async_wait([&](const boost::system::error_code& ec) { OnTimerExpired(ec); });
}
