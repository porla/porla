#include "timers.hpp"

#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>

#include "../plugins/plugin.hpp"

using porla::Lua::Packages::Timers;

class Timer
{
public:
    explicit Timer(boost::asio::io_context& io, sol::table args)
        : m_timer(io)
        , m_args(std::move(args))
    {
        Next();
    }

    Timer(const Timer&) = delete;
    Timer(Timer&&) = delete;
    Timer& operator=(const Timer&) = delete;
    Timer& operator=(Timer&&) = delete;

    ~Timer()
    {
        m_should_cancel = true;
        m_timer.cancel();
    }

    void Cancel()
    {
        m_should_cancel = true;
        m_timer.cancel();
    }

private:
    void Next()
    {
        const int milliseconds = m_args["interval"];

        m_timer.expires_from_now(boost::posix_time::milliseconds(milliseconds));
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

    boost::asio::deadline_timer m_timer;
    sol::table m_args;
    bool m_should_cancel;
};

void Timers::Register(sol::state& lua)
{
    auto timer_type = lua.new_usertype<Timer>(
        "porla.Timer",
        sol::no_constructor,
        "cancel", &Timer::Cancel);

    lua["package"]["preload"]["timers"] = [](sol::this_state s)
    {
        sol::state_view lua{s};
        sol::table timers = lua.create_table();

        timers["new"] = [](sol::this_state s, const sol::table& args)
        {
            sol::state_view lua{s};
            const auto& options = lua.globals()["__load_opts"].get<const Plugins::PluginLoadOptions&>();
            return std::make_unique<Timer>(options.io, args);
        };

        return timers;
    };
}
