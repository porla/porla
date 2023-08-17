#include "../packages.hpp"

#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <croncpp.hpp>

#include "../plugin.hpp"

using porla::Lua::Packages::Cron;

class CronSchedule
{
public:
    CronSchedule(boost::asio::io_context &io, sol::table args)
        : m_args(std::move(args))
        , m_timer(io)
        , m_should_cancel(false)
    {
        Next();
    }

    ~CronSchedule()
    {
        BOOST_LOG_TRIVIAL(debug) << "Destroying cron schedule";

        m_should_cancel = true;
        m_timer.cancel();
    }

    void Cancel()
    {
        BOOST_LOG_TRIVIAL(debug) << "Cancelling timer";

        m_should_cancel = true;
        m_timer.cancel();
    }

private:
    void Next()
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

    void OnTimerExpired(const boost::system::error_code &ec)
    {
        if (ec)
        {
            if (ec == boost::system::errc::operation_canceled)
            {
                return;
            }

            BOOST_LOG_TRIVIAL(error) << "Timer error: " << ec.message();

            return;
        }

        if (m_args["callback"].is<sol::function>())
        {
            try
            {
                m_args["callback"]();
            }
            catch (const sol::error& err)
            {
                BOOST_LOG_TRIVIAL(error) << "Error when invoking callback: " << err.what();
            }
        }

        if (!m_should_cancel) Next();
    }

    sol::table m_args;
    bool m_should_cancel;
    boost::asio::deadline_timer m_timer;
};

void Cron::Register(sol::state& lua)
{
    auto type = lua.new_usertype<CronSchedule>(
        "cron.Schedule",
        sol::no_constructor,
        "cancel", &CronSchedule::Cancel);

    lua["package"]["preload"]["cron"] = [](sol::this_state s)
    {
        sol::state_view lua{s};
        sol::table cron = lua.create_table();

        cron["schedule"] = [](sol::this_state s, const sol::table& args)
        {
            sol::state_view lua{s};
            const auto& options = lua.globals()["__load_opts"].get<const PluginLoadOptions&>();
            return std::make_shared<CronSchedule>(options.io, args);
        };

        return cron;
    };
}
